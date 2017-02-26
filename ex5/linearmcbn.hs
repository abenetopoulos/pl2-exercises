{-# OPTIONS_GHC -static -rtsopts -O2 -optc-O2 -fprof-auto #-}
import Data.Char
import qualified Data.Map.Strict as Map

type X  =  Char
data Val = Character X | Complex (Val -> Denot) | DummyVal
type Denot = State -> (Val, State)
type State = ([X], [X])
type Env = Map.Map Char Val
data E  =  Literal Char | Write E E | Read E
        |  Variable X | Lambda X E | Application E E deriving (Show)

next (x : r) = [(x, r)]
next _ = []

instance Read E where
    readsPrec p s =
      [(Literal c, r)        |  ('#', t) <- next s, (c, r) <- next t] ++
      [(Write e1 e2, r)  |  ('+', t1) <- next s,
                            (e1, t2) <- readsPrec 0 t1,
                            (e2, r) <- readsPrec 0 t2] ++
      [(Read e, r)       |  ('-', t) <- next s, (e, r) <- readsPrec 0 t] ++
      [(Variable x, r)        |  (x, r) <- next s, isLower x] ++
      [(Lambda x e, r)      |  ('/', t1) <- next s,
                            (x, t2) <- next t1, isLower x,
                            (e, r) <- readsPrec 0 t2] ++
      [(Application e1 e2, r)    |  ('@', t1) <- next s,
                            (e1, t2) <- readsPrec 0 t1,
                            (e2, r) <- readsPrec 0 t2]

main = do
    contents <- getContents
    let
        (phrase : input : _) = lines contents
        e = read phrase :: E
        d = sem e Map.empty
    interpret d input
    return ()

interpret d input =
    let
        tup = d (input, "")
    in
        putStrLn (reverse (snd (snd tup)))

sem :: E -> Map.Map X Val -> Denot
sem phrase env =
    case phrase of
    Literal c ->
        let
            functor s =
                (Character c, s)
        in
            functor
    Write e1 e2 ->
        let
            e1Functor = sem e1 env

            functor s =
                let
                    (e1Res, s') = e1Functor s
                    toOutput =
                        case e1Res of
                        Character c -> c
                        Complex f -> '-'
                    (inp, outp) = s'
                in
                    (e1Res, (inp, toOutput : outp))

            e2Functor = sem e2 env

            functor' s =
                let
                    (resOfOutput, s') = functor s
                in
                    e2Functor s'
        in
            functor'
    Read e ->
        let
            eFunctor = sem e env

            functor s =
                let
                    (inp, outp) = s
                    i : is = inp
                    (eVal, eState) = eFunctor (is, outp) --do we care about e's state?
                in
                    case eVal of
                    Complex f -> f (Character i) (is, outp)
                    Character x -> (eVal, eState)
        in
            functor
    Variable x ->
        let
            functor s =
                case (Map.lookup x env) of
                Nothing -> error "not in table"
                Just e ->
                    case e of
                    Complex f -> f DummyVal s
                    Character _ -> (e, s)
        in
            functor
    Lambda arg body ->
        let
            argFunctor v =
                let
                    newEnv = Map.insert arg v env
                    bodyFunctor = sem body newEnv
                in
                    bodyFunctor

            argVal = Complex argFunctor

            functor s =
                (argVal, s)
        in
            functor
    Application e1 e2 ->
        let
            e1Functor = sem e1 env
            e2Functor = sem e2 env
            e2Arg v = e2Functor
            functor s =
                let
                    res @ (e1Val, e1State) = e1Functor s
                in
                    case e1Val of
                    Complex f -> f (Complex e2Arg) e1State
                    Character x -> res
        in
            functor

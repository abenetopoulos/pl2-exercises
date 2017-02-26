{- OPTIONS_GHC -static -rtsopts -O2 -optc-O2 -fprof-auto #-}
import System.IO as SIO
import qualified Data.ByteString.Lazy.Char8 as L
import qualified Data.Map.Strict as Map
import Data.List

data Term = Variable String | Lambda String Term | Application Term Term deriving (Show)
data Type = TypeError | BasicType Int | Function Type Type deriving (Show, Eq)
type Environment = Map.Map String Type

main = do
    s <- L.hGetContents SIO.stdin
    let termStrings = get_terms (tail (L.lines s))
        terms = parse_terms termStrings
        logicalTypes = map (\x -> infer Map.empty x 0) terms
        actualTypes = map (\(t, c, _) -> unify t c) logicalTypes
    mapM print logicalTypes
    mapM display_type actualTypes
    return ()

get_terms s =
    reverse (foldl' (\a x -> (L.unpack x) : a) [] s)

parse_terms s =
    let
        get_var_name s ch =
            let aux a = ('a' <= a && a <= 'z') || ('0' <= a && a <= '9') || ('A' <= a && a <= 'Z')
                second = if ch == '.' then tail (tail (dropWhile aux s)) else tail (dropWhile aux s)
            in
                ((takeWhile aux s), second)

        parse t =
            case (head t) of
            '(' -> --application or lambda
                let restOfTerm = tail t in
                case (head restOfTerm) of
                '\\' -> --lambda
                    let (varName, body) = get_var_name (tail restOfTerm) '.'
                        (bodyTerm, rest) = parse body
                     in
                        ((Lambda varName bodyTerm), (tail rest))
                _    -> --application
                    let (firstTerm, restOfApplication) = parse restOfTerm
                        (secondTerm, rest) = parse restOfApplication
                    in
                        ((Application firstTerm secondTerm), tail rest)
            _   -> --variable
                let (varName, rest) = get_var_name t ' '
                in
                    (Variable varName, rest)

        aux t =
            let (term, _) = parse t
            in
                term
    in
        map aux s

infer env term counter =
    case term of
    Variable varName ->
        case (Map.lookup varName env) of
        Nothing -> error "unbound variable"
        Just t -> (t, [], counter)
    Lambda varName body ->
        let argType = BasicType counter
            newEnv = Map.insert varName argType env
            (bodyType, constraints, newCounter) = infer newEnv body (counter + 1)
        in
            (Function argType bodyType, constraints, newCounter)
    Application firstTerm secondTerm ->
        let (firstType, firstConstraints, firstCounter) = infer env firstTerm counter
            (secondType, secondConstraints, newCounter) = infer env secondTerm firstCounter
            retType = BasicType newCounter
            newConstraint = (firstType, Function secondType retType)
        in
            (retType, newConstraint : (firstConstraints ++ secondConstraints), newCounter + 1)

unify typ [] = typ
unify typ (c@(ls, rs) : cs) =
    let
        function_type t =
            case t of
            Function _ _ -> True
            _ -> False

        basic_type t =
            case t of
            BasicType _ -> True
            _ -> False

        q `in_term` t =
            case t of
            Function arg body ->
                (q `in_term` arg) || (q `in_term` body)
            BasicType bt ->
                case q of
                BasicType qt -> t == q
                _ -> error "mismatch"

        modify sourcet endt = --modify second arg based on first arg
            let
                BasicType s = sourcet
            in
                case endt of
                Function arg body ->
                    Function (modify sourcet arg) (modify sourcet body)
                BasicType e ->
                    if e > s then
                        BasicType (e - 1)
                    else
                        BasicType e

        replace_in_sig sourcet endt typ =
            case typ of
            BasicType e ->
                if (sourcet == typ) then
                    endt
                else
                    let
                        BasicType s = sourcet
                    in
                        if e > s then
                            BasicType (e - 1)
                        else
                            typ
            Function arg body ->
                Function (replace_in_sig sourcet endt arg) (replace_in_sig sourcet endt body)

        replace_in_constraints sourcet endt cs =
            let
                aux = replace_in_sig sourcet endt
            in
                map (\(l, r) -> (aux l, aux r)) cs

        get_min left right =
            let
                BasicType l = left
                BasicType r = right
            in
                if l < r then
                    (left, right)
                else
                    (right, left)

    in
        if ls == rs then
            unify typ cs
        else if (basic_type ls && basic_type rs) then
            let
                (minVal, oldVal) = get_min ls rs
                newType = replace_in_sig oldVal minVal typ
                newConstraintSet = replace_in_constraints oldVal minVal cs
            in
                newConstraintSet `seq` unify newType newConstraintSet
        else if (function_type ls && function_type rs) then
            let
                Function lsArg lsBody = ls
                Function rsArg rsBody = rs
            in
                unify typ ((lsArg, rsArg) : (lsBody, rsBody) : cs)
        else if (not (function_type ls)) && (not (ls `in_term` rs)) then
            let
                newRs = modify ls rs
                newType = replace_in_sig ls newRs typ
                newConstraintSet = replace_in_constraints ls newRs cs
            in
                newConstraintSet `seq` unify newType newConstraintSet
        else if (not (function_type rs)) && (not (rs `in_term` ls)) then
            let
                newLs = modify rs ls
                newType = replace_in_sig rs newLs typ
                newConstraintSet = replace_in_constraints rs newLs cs
            in
                newConstraintSet `seq` unify newType newConstraintSet
        else
            TypeError

display_type ty = do
    pretty_print ty
    putChar '\n'

pretty_print ty =
    case ty of
    BasicType i ->
        putStr ("@" ++ (show i))
    Function lt@(Function _ _) rt ->
        do
            putStr "("
            pretty_print lt
            putStr ")"
            putStr " -> "
            pretty_print rt
    Function lt rt ->
        do
            pretty_print lt
            putStr " -> "
            pretty_print rt
    TypeError ->
        putStr "type error"

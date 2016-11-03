import System.IO as SIO
import Data.ByteString.Lazy.Char8 as L


main = do --use bleeding 'interact' mate
    s <- L.hGetContents SIO.stdin
    let sep = L.lines s
        tups = extract_tuples sep
--    let listOfStrings = lines s
--        r = map extract_tuples listOfStrings
--        rs = print_shit r
    mapM_ print tups
    return ()

extract_tuples :: [ByteString] -> [(Int, Int)]
extract_tuples = Prelude.map (\x -> case func x of
                            Nothing -> (-1, -1)
                            Just s -> s) 
    where
        func s = 
            --L.putStrLn s >>
            case L.readInt s of
            Nothing -> Nothing
            Just (fNode, rest) ->
                case L.readInt rest of
                Nothing -> Nothing
                Just (sNode, more) -> Just (fNode, sNode)
func s = 
    case L.readInt s of
    Nothing -> Nothing
    Just (fNode, rest) ->
        case L.readInt rest of
        Nothing -> Nothing
        Just (sNode, more) -> Just (fNode, sNode)

--extract_tuples :: String -> (a , a)
--extract_tuples = map (\x -> fst . head $ x) . map param_reads . words
--
--param_reads x = reads x :: [(Int, String)]
--
--print_shit x = map (\e -> show . head $ e) x

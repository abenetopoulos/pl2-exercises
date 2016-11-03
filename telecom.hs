import System.IO as SIO
import qualified Data.ByteString.Lazy.Char8 as L
import qualified Data.Map.Strict as Map

type Graph = Map.Map Int [Int]

main = do 
    s <- L.hGetContents SIO.stdin
    let sep = L.lines s
        tups = extract_tuples sep
        desc = head tups
        graph = generate_graph (tail tups)
        -- now that we have the graph, we need to perform the search
    mapM_ print tups
    print graph
    return ()

extract_tuples :: [L.ByteString] -> [(Int, Int)]
extract_tuples = 
    Prelude.map (\x -> case func x of
                       Nothing -> (-1, -1)
                       Just s -> s) 
    where
        func s = 
            case (L.readInt s) of
            Nothing -> Nothing
            Just (fNode, rest) ->
                case (L.readInt . L.tail $ rest) of
                Nothing -> Just (fNode, -1)
                Just (sNode, more) -> Just (fNode, sNode)

generate_graph :: [(Int, Int)] -> Graph
generate_graph x = graph
    where
        aux m (l, r) =
            let firstL = 
                    case (Map.lookup l m) of
                    Nothing -> [r]
                    Just lst -> (r : lst)
                firstNew = Map.insert l firstL m
                secondL = 
                    case (Map.lookup r firstNew) of
                    Nothing -> [l]
                    Just lst -> (l : lst)
            in Map.insert r secondL firstNew
        graph = foldl aux Map.empty x

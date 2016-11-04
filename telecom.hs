{-# OPTIONS_GHC -O2 -optc-O2 #-}

import System.IO as SIO
import qualified Data.ByteString.Lazy.Char8 as L
import qualified Data.Map.Strict as Map

type Graph = Map.Map Int [Int]
type Parent = Int
type LowPoint = Int
type StartTime = Int
type EndTime = Int
--type Times = Map.Map Int ((Int, Int), Int)
type Times = Map.Map Int ((StartTime, EndTime), Parent, LowPoint)

main = do
    s <- L.hGetContents SIO.stdin
    let sep = L.lines s
        tups = extract_tuples sep
        desc = head tups
        graph = generate_graph (tail tups)
        searched = dfs graph
    --mapM_ print tups
    --print graph
    print searched
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

dfs :: Graph -> Times
dfs g =
    times
    where
        aux (time, timesMap) k v =
            case (Map.lookup k timesMap) of
            Nothing ->
                let newTimesMap = Map.insert k ((time, -1), k, 1) timesMap
                    (finalTime, _, auxTimes, lowPoint) = search' v k (time + 1) g newTimesMap (time + 1)
                    latestTimes = Map.insert k ((time, finalTime), k, 1) auxTimes
                in (finalTime + 1, latestTimes)
            Just _ -> (time, timesMap)
        times = snd (aux (1, Map.empty) 1 (get_elem g))
        --times = snd (Map.foldlWithKey' (\(a, b) k v -> aux (a, b) k v) (1, Map.empty) g) --actually, since the graph is connected to begin with,
                                                                                         --we don't really need to fold the entire Map, we just
                                                                                         --need to take the first element and run search from there
--search [] parent time graph times = (time, graph, times)
--search (n : ns) parent time graph times =
--    case (Map.lookup n times) of
--    Nothing ->
--        let tempTimes = Map.insert n ((time, -1), parent) times
--        in
--            case (Map.lookup n graph)  of
--            Nothing -> error "Could not find node neighbour list in graph"
--            Just nNeighbours ->
--                let
--                    (finalTime, _, auxTimes) = search nNeighbours n (time + 1) graph tempTimes
--                    latestTimes = finalTime `seq` Map.insert n ((time, finalTime), parent) auxTimes
--                in
--                    latestTimes `seq` search ns parent (finalTime + 1) graph latestTimes
--    Just entry ->
--        search ns parent time graph times

search' [] parent time graph times lowPoint = (time, graph, times, lowPoint)
search' (n : ns) parent time graph times lowPoint =
    case (Map.lookup n times) of
    Nothing ->
        let tempTimes = Map.insert n ((time, -1), parent, -1) times
        in
            case (Map.lookup n graph)  of
            Nothing -> error "Could not find node neighbour list in graph"
            Just nNeighbours ->
                let
                    (finalTime, _, auxTimes, newLowPoint) = search' nNeighbours n (time + 1) graph tempTimes (time + 1)
                    actualLowPoint = min lowPoint newLowPoint
                    latestTimes = finalTime `seq` Map.insert n ((time, finalTime), parent, actualLowPoint) auxTimes
                in
                    latestTimes `seq` search' ns parent (finalTime + 1) graph latestTimes (finalTime + 1)
    Just ((start, end), _, _) ->
        if parent == n then
            search' ns parent time graph times lowPoint
        else
            search' ns parent time graph times start

get_elem g = elems
    where
        elems =
            case (Map.lookup 1 g) of
            Nothing -> []
            Just e -> e

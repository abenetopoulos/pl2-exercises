{-# OPTIONS_GHC -O2 -optc-O2 #-}
import System.IO as SIO
import qualified Data.ByteString.Lazy.Char8 as L
import qualified Data.Map.Strict as Map
import Data.List

type Graph = Map.Map Int [Int]
type Parent = Int
type LowPoint = Int
type StartTime = Int
type EndTime = Int
--type Times = Map.Map Int ((StartTime, EndTime), Parent, LowPoint)
type Times = Map.Map Int ((StartTime, EndTime), Parent)
--type Times = Map.Map Int ((StartTime, EndTime), Parent, Bool)

main = do
    s <- L.hGetContents SIO.stdin
    let sep = L.lines s
        graph = generate_graph (tail sep)
--        tups = extract_tuples sep
--        desc = head tups
--        graph = generate_graph (tail tups)
        --graph = Map.fromListWith (++) (tail tups)
        --searched = dfs graph
        --numCritical = Map.size (Map.filter (\(_, _, isCritical) -> isCritical) searched)
    --mapM_ print tups
    --print graph
    --print searched
    --print numCritical
    print (length tups)
    graph `seq` return ()

extract_tuples' :: [L.ByteString] -> [(Int, [Int])]
extract_tuples' =
    reverse . foldl' (\acc x -> case func x of
                       Nothing -> acc
                       Just (sl, sr) -> sl : (sr : acc)) []
    where
        func s =
            case (L.readInt s) of
            Nothing -> Nothing
            Just (fNode, rest) ->
                case (L.readInt . L.tail $ rest) of
                Nothing -> Just ((fNode, [-1]), (-1, [fNode]))
                Just (sNode, more) -> Just ((fNode, [sNode]), (sNode, [fNode]))

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
                secondL =
                    case (Map.lookup r m) of
                    Nothing -> [l]
                    Just lst -> (l : lst)
            in Map.insert r secondL $ Map.insert l firstL m
        graph = foldl' aux Map.empty x

generate_graph' [] acc = acc
generate_graph' (x:xs) acc =
    newAcc `seq` generate_graph' xs newAcc
    where
        l = fst x
        r = snd x
        fLst =
            case (Map.lookup l acc) of
                Nothing -> [r]
                Just lst -> (r : lst)
        newAcc = Map.insert l fLst acc

dfs :: Graph -> Times
dfs g =
    times
    where
        aux (time, timesMap) k v =
            case (Map.lookup k timesMap) of
            Nothing ->
                --let newTimesMap = Map.insert k ((time, -1), k, 1) timesMap
                --let newTimesMap = Map.insert k ((time, -1), k, False) timesMap
                let newTimesMap = Map.insert k ((time, -1), k) timesMap
                    --(finalTime, _, auxTimes, lowPoint) = search v k (time + 1) g newTimesMap (time + 1)
                    (finalTime, _, auxTimes) = vanilla_search v k (time + 1) g newTimesMap
                    --latestTimes = Map.insert k ((time, finalTime), k, 1) auxTimes
                    latestTimes = Map.insert k ((time, finalTime), k) auxTimes
                    --rootIsCritical = check_root times
                    --latestTimes = Map.insert k ((time, finalTime), k, rootIsCritical) auxTimes
                in (finalTime + 1, latestTimes)
            Just _ -> (time, timesMap)
        times = snd (aux (1, Map.empty) 1 (get_elem g))
        --times = snd (Map.foldlWithKey' (\(a, b) k v -> aux (a, b) k v) (1, Map.empty) g) --actually, since the graph is connected to begin with,
                                                                                         --we don't really need to fold the entire Map, we just
                                                                                         --need to take the first element and run search from there
check_root times = (Map.size childrenOfRoot) > 2
    where
        childrenOfRoot = Map.filter (\(_, p, _) -> if p == 1 then True else False) times

--NOTE: lowPoint should always contain the actual lowPoint of the current level. So when a node calls search' on its children
--it should get back the lowPoint we want, meaning the min of the start times of its neighbours (excluding its parent's) and
--the lowpoint of its neighbours' children
search [] parent time graph times lowPoint = (time, graph, times, lowPoint)
search (n : ns) parent time graph times lowPoint =
    case (Map.lookup n times) of
    Nothing ->
        let tempTimes = Map.insert n ((time, -1), parent, -1) times
        in
            case (Map.lookup n graph)  of
            Nothing -> error "Could not find node neighbour list in graph"
            Just nNeighbours ->
                let
                    (finalTime, _, auxTimes, newLowPoint) = search nNeighbours n (time + 1) graph tempTimes (time + 1)
                    actualLowPoint = min time newLowPoint
                    latestTimes = finalTime `seq` Map.insert n ((time, finalTime), parent, actualLowPoint) auxTimes
                in
                    latestTimes `seq` search ns parent (finalTime + 1) graph latestTimes (min lowPoint newLowPoint)
    Just ((start, end), _, _) -> --this bit could be a bit slow
        case (Map.lookup parent times) of
        Nothing -> error "Could not find parent node in times"
        Just (_, par, _) ->
            if par == n then
                search ns parent time graph times lowPoint
            else
                search ns parent time graph times start

search' [] parent time graph times lowPoint maxLowPoint = (time, graph, times, lowPoint, maxLowPoint)
search' (n : ns) parent time graph times lowPoint maxLowPoint =
    case (Map.lookup n times) of
    Nothing ->
        let tempTimes = Map.insert n ((time, -1), parent, False) times
        in
            case (Map.lookup n graph)  of
            Nothing -> error "Could not find node neighbour list in graph"
            Just nNeighbours ->
                let
                    (finalTime, _, auxTimes, newLowPoint, newMaxLowPoint) = search' nNeighbours n (time + 1) graph tempTimes (time + 1) (time)
                    actualLowPoint = min time newLowPoint
                    isCut = if finalTime == time + 1 then False
                            else
                                if newMaxLowPoint >= time then True
                                else False
                    latestTimes = finalTime `seq` Map.insert n ((time, finalTime), parent, isCut) auxTimes
                in
                    latestTimes `seq` search' ns parent (finalTime + 1) graph latestTimes (min lowPoint newLowPoint) (max maxLowPoint actualLowPoint)
    Just ((start, end), _, _) ->
        case (Map.lookup parent times) of
        Nothing -> error "Could not find parent node in times"
        Just (_, par, _) ->
            if par == n then
                search' ns parent time graph times lowPoint maxLowPoint
            else
                search' ns parent time graph times start maxLowPoint

vanilla_search [] parent time graph times = (time, graph, times)
vanilla_search (n : ns) parent time graph times =
    case (Map.lookup n times) of
    Nothing ->
        let tempTimes = Map.insert n ((time, -1), parent) times --at any given time, there might be _a lot_ of copies of 'times' in memory...
        in
            case (Map.lookup n graph)  of
            Nothing -> error "Could not find node neighbour list in graph"
            Just nNeighbours ->
                let
                    (finalTime, _, auxTimes) = vanilla_search nNeighbours n (time + 1) graph tempTimes
                    latestTimes = finalTime `seq` Map.insert n ((time, finalTime), parent) auxTimes --consider using update maybe?
                in
                    latestTimes `seq` vanilla_search ns parent (finalTime + 1) graph latestTimes
    Just ((start, end), _) -> --this bit could be a bit slow
        case (Map.lookup parent times) of
        Nothing -> error "Could not find parent node in times"
        Just (_, par) ->
            vanilla_search ns parent time graph times

search_on_peds arg = Map.empty

get_elem g = elems
    where
        elems =
            case (Map.lookup 1 g) of
            Nothing -> []
            Just e -> e

--foldl' f z [] = z
--foldl' f z (x:xs) = let z' = z `f` x
--                    in seq z' $ foldl' f z' xs

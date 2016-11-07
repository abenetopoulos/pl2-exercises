{-# OPTIONS_GHC -static -rtsopts -O2 -optc-O2 #-}
import System.IO as SIO
import qualified Data.ByteString.Lazy.Char8 as L
import qualified Data.Map.Strict as Map
import Data.List

type Graph = Map.Map Int [Int]
type Parent = Int
type LowPoint = Int
type StartTime = Int
type EndTime = Int
type Times = Map.Map Int ((StartTime, EndTime), Parent, Bool)

main = do
    s <- L.hGetContents SIO.stdin
    let sep = L.lines s
        graph = generate_graph (tail sep)
        searched = dfs graph
    print (Map.size (Map.filter (\(_,_,b) -> b) searched))
    return ()

generate_graph :: [L.ByteString] -> Graph
generate_graph s =
    foldl' aux Map.empty s
    where
        func bs =
            case (L.readInt bs) of
            Nothing -> (-1,-1)
            Just (fNode, rest) ->
                case (L.readInt . L.tail $ rest) of
                Nothing -> (-1,-1)
                Just (sNode, more) -> (fNode, sNode)
        change_value k v m =
            case (Map.lookup k m) of
            Nothing -> (Map.insert k [v] m)
            Just l -> (Map.adjust (v :) k m)
        aux m bs =
            let (l, r) = func bs
            in
                change_value l r $ change_value r l m

dfs :: Graph -> Times
dfs g =
    times
    where
        aux (time, timesMap) k v =
            case (Map.lookup k timesMap) of
            Nothing ->
                let newTimesMap = Map.insert k ((time, -1), k, False) timesMap
                    (finalTime, auxTimes, _, _) = search_on_peds v k (time + 1) g newTimesMap (time + 1) 0
                    latestTimes = Map.insert k ((time, finalTime), k, (check_root auxTimes)) auxTimes
                in (finalTime + 1, latestTimes)
            Just _ -> (time, timesMap)
        times = snd (aux (1, Map.empty) 1 (get_elem g))

check_root times = (Map.size childrenOfRoot) > 2
    where
        childrenOfRoot = Map.filter (\(_, p, _) -> if p == 1 then True else False) times

search [] _ time _ times candidateLowPoint maxLowPoint = (time, times, candidateLowPoint, maxLowPoint)
search (n : ns) parent time graph times candidateLowPoint maxLowPoint =
    case (Map.lookup n times) of
    Nothing ->
        let tempTimes = Map.insert n ((time, -1), parent, False) times
        in
            case (Map.lookup n graph) of
            Nothing -> error "n not in graph"
            Just neighbours ->
                let (finalTime, auxTimes, childrenLowPoint, maxChildrenLowPoint) = search neighbours n (time + 1) graph tempTimes (time + 1) 0
                    nLowPoint = min time childrenLowPoint
                    isCut =
                        if (finalTime == time + 1) then False
                        else
                            if maxChildrenLowPoint >= time then True else False
                    latestTimes = Map.adjust (\((t, _), p, _) -> ((time, finalTime), parent, isCut)) n auxTimes
                in latestTimes `seq` search ns parent (finalTime + 1) graph latestTimes (min candidateLowPoint nLowPoint) (max maxLowPoint nLowPoint)
    Just ((start, _), _, _) ->
        case (Map.lookup parent times) of
        Nothing -> error "parent should have been in the times map"
        Just (_, par, _) ->
            if (par == n) then
                --we found our parent, proceed
                search ns parent time graph times candidateLowPoint maxLowPoint
            else
                --parent has a back edge, we should signify this
                search ns parent time graph times (min start candidateLowPoint) maxLowPoint


get_elem g = elems
    where
        elems =
            case (Map.lookup 1 g) of
            Nothing -> []
            Just e -> e

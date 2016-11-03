import Data.Char(toUpper)

main =
    putStrLn "Hello there. what is your name?" >>
    getLine >>=
    putStrLn . (++) "welcome "

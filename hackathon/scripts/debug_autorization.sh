#!/bin/bash

#cleos --host 192.168.100.42 --port 8888 --wallet-host 192.168.100.42 --wallet-port 8888

#default
cleos wallet open

cat << ENDOFPASS | cleos wallet unlock
PW5K9gJRKkSPQ34ijpaXU4YqQPeVCF6Pek6skmZ9G9gi41mtVUzKH
ENDOFPASS

#cleos wallet import 5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3

#currency
cleos wallet open -n currency

cat << ENDOFPASS | cleos wallet unlock -n currency
PW5K4rP1WE9dbAKsr8pwQawhsuhQJps2wVZMTbnxYfQcBgrWkMbbS
ENDOFPASS

#cleos wallet import -n currency 5J73q1Dxrpn3b5dYdtSTvcCVBcwZSvZ7x6FGsX64ZbAfaD2RXsk
#cleos wallet import -n currency 5JjDzaNJrpqxw2Upjyw9xUqRGUHq3heYfC9AdQPQowBEWGvwkbU

cleos create account eosio currency EOS7drQWvc7Mn7NK2PivjbAqLXMyBpvSNnZWnZC3CS61g1dhVK57o EOS8KLWY5tcczw6tTkk4UhfeZJrES7ECiFZAkChcR2mwsFcArURn7

#steepshot
#cleos wallet open -n steepshot

#cat << ENDOFPASS | cleos wallet unlock -n steepshot
#PW5KBRjzUZynGC3826RgtE9f2mrSy2aj6kjEeTbhuPnXK9MqtRCj8
#ENDOFPASS

#cleos wallet import -n steepshot 5J1sgWdayyR9dKEd9xiypReXLpmD7p5XqTjiuFJGjDJMk1avkUr
#cleos wallet import -n steepshot 5JCCWxS2uLWHwCqhm4exdhNwyxuKuvMnmukAxgk2wW5RPG379Gx

#cleos create account eosio steepshot EOS5PbBDvY9Q5FSQqwLbHiqmtj9s3tKSsupXbKg4TY3qPNQkcoJSw EOS8jc1SKpWrVc3h87s4sXNGt4DDSwtx4oRYRZezS2uBKWuKuTUNe

#echo "----------DEPLOY CONTRACT----------"
#cleos set contract currency /root/buildQt/EOS/contracts/steepshot/
cleos set contract currency /root/buildQt/EOS/contracts/hackathon/

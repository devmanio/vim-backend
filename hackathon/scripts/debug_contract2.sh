#!/bin/bash

#cleos --host 192.168.100.42 --port 8888 --wallet-host 192.168.100.42 --wallet-port 8888
#cleos --host 188.166.103.60 --port 8888 --wallet-host 188.166.103.60 --wallet-port 8888

echo "----------DEPLOY CONTRACT----------"
#cleos set contract currency /root/buildQt/EOS/contracts/hackathon/ -p currency

cleos push action currency init '{"autoruzation": "1"}' -p currency@active
cleos get table currency currency infotable

# test add account
echo "----------ADD ACCOUNT----------"
cleos push action currency addaccount '{"account": "test1"}' -p currency@active
cleos push action currency addaccount '{"account": "test2"}' -p currency@active
cleos push action currency addaccount '{"account": "test3"}' -p currency@active
#cleos push action currency delaccount '{"account": "test2"}' -p currency@active
echo "----------RESULT ADD ACCOUNT----------"
cleos get table currency currency emisstable

# test fund base account
#echo "----------FUND BASE ACCOUNT----------"
#cleos push action currency fundbaseacc '{"account": "currency", "hash": "1"}' -p currency@active
#echo "----------RESULT FUND BASE ACCOUNT----------"
#cleos get table currency currency accounts

# test fund account
echo "----------FUND ACCOUNT----------"
cleos push action currency fundaccount '{"account": "currency", "balance": "100.0000 VIM"}' -p currency@active 
#cleos push action currency fundaccount '{"account": "test2", "balance": "2000.VIM"}' -p currency@active
echo "----------RESULT FUND ACCOUNT----------"
cleos get table currency currency accounts

# test transfer
echo "----------TRANSFER----------"
cleos push action currency transfer '{"from": "currency", "to": "test1" , "amount": "1.0000 VIM"}' -p currency@active
echo "----------RESULT TRANSFER----------"
cleos get table currency currency accounts
cleos get table currency test1 accounts

# test emission
echo "----------EMISSION----------"
cleos push action currency emission '{"hash": "1"}' -p currency@active 
echo "----------RESULT EMISSION----------"
 cleos get table currency currency accounts
 cleos get table currency test1 accounts
 cleos get table currency test2 accounts
 cleos get table currency test3 accounts

# test up vote
echo "----------UP VOTE----------"
cleos push action currency upvote '{"uuid_post": "1", "account": "test1"}' -p currency@active
cleos push action currency upvote '{"uuid_post": "1", "account": "test2"}' -p currency@active
cleos push action currency upvote '{"uuid_post": "1", "account": "test3"}' -p currency@active
cleos push action currency upvote '{"uuid_post": "2", "account": "test2"}' -p currency@active
echo "----------RESULT UP VOTE----------"
cleos get table currency currency votetable

# test down vote
echo "----------DOWN VOTE----------"
cleos push action currency downvote '{"uuid_post": "1", "account": "test2"}' -p currency@active
echo "----------RESULT DOWN VOTE----------"
cleos get table currency currency votetable

# test up vote
echo "----------CREATE POST----------"
cleos push action currency createpost '{"account": "test1", "uuid_post": "1", "url": "1", "hash": "11" }' -p currency@active
cleos push action currency createpost '{"account": "test2", "uuid_post": "2", "url": "2", "hash": "12"}' -p currency@active
cleos push action currency createpost '{"account": "test3", "uuid_post": "3", "url": "3", "hash": "13"}' -p currency@active
cleos push action currency createpost '{"account": "test1", "uuid_post": "4", "url": "4", "hash": "14"}' -p currency@active
echo "----------RESULT CREATE POST----------"
cleos get table currency currency posttable






cleos get table currency currency infotable

























a=$(cat ./a.txt)
b=$(cat ./b.txt)
./plcs-64 $1 "$a" "$b"
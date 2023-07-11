a=$(cat ./a.txt)
b=$(cat ./b.txt)
time ./plcs-64 1 "$a" "$b"
time ./plcs-64 5 "$a" "$b"
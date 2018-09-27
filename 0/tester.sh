#!/bin/bash
./project? "$1" "$2" < "$3" > "$3.me"
diff "examples/$3.out" "$3.me"
rm "$3.me"

#!/usr/bin/env bash

if [[ ! -f "$1" ]] || [[ ! -f "$2" ]]
then
  echo "Usage: $0 <pcap file> <pcap file>"
  exit 1
fi

output=$(mktemp)
expected=$(mktemp)

function cleanup_exit {
  rm $output $expected
  exit 1
}
trap cleanup_exit EXIT ERR

sudo tcpdump -t -n -vvv -x -e -r $1 > $output   2>/dev/null
sudo tcpdump -t -n -vvv -x -e -r $2 > $expected 2>/dev/null

if cmp --silent $output $expected
then
  echo "$1 and $2 are identical"
else
  (
  echo "$1 and $2 are different:"
  echo "== Diff of outputs =="
  echo -n "|                           OUTPUT                                             |"
  echo    "|                       EXPECTED OUTPUT                                        |"
  sdiff -s -d -w160 $output $expected
  echo
  echo

  echo "== Side by side comparison of outputs =="
  echo -n "|                           OUTPUT                                             |"
  echo    "|                       EXPECTED OUTPUT                                        |"
  sdiff -d -w160 $output $expected
  echo
  echo
  ) | less
fi

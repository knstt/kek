#!/bin/bash
# Send input to the program character by character
(
  sleep 0.5
  printf "h"
  sleep 0.1
  printf "e"
  sleep 0.1
  printf "l"
  sleep 0.1
  printf "l"
  sleep 0.1
  printf "o"
  sleep 0.2
  printf "q"
  sleep 0.2
) | timeout 5 ./main

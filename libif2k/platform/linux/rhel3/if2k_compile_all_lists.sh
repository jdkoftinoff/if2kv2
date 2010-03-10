#! /bin/bash

echo Precompiling built-in internet filter database:

for i in /etc/if2k/predb/*phr.txt
do
  echo "$i -> $i.pre"
  if2k_mini_compile_list alpha "$i" "$i.pre" 0 ""
done

for i in /etc/if2k/predb/*url.txt
do
  echo "$i -> $i.pre"
  if2k_mini_compile_list url "$i" "$i.pre" 0 ""
done

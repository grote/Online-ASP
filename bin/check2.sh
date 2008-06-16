#!/bin/bash

n=$1
shift
echo $1
gringo2 -l $* | clasp -n $n | grep -C 1 '^Answer: [0-9]*$' | grep -v '^Answer: [0-9]*$' | tail -n +2  > answers.txt
l=$(wc -l < answers.txt)
echo "found $l solutions"
if [[ $l == 0 ]]; then
	if lparse $* | clasp | grep "Models      : 0"; then
		echo True
	else
		echo False
	fi
else
	lparse $* > compare.sm
	g=$(cat answers.txt | awk 'BEGIN { while((getline str) > 0) { gsub(/\(/, "\\(", str); gsub(/\)/, "\\)", str); gsub(/\"/, "\\\"", str); system("testsm compare.sm " str " | smodels 1"); } }' | grep "True" | wc -l)
	if [[ $l == $g ]]; then
		echo True
	else
		echo False
	fi
fi
	

#!/bin/bash
sed -e 's/ :- \././g' -e 's/{/[/g' -e 's/}/]/g' -e 's/sum //g'

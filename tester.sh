#!/bin/bash
function diffs() {
    diff "${@:4}" <(cat sample_output/result-"$1"-"$2"-input_"$3") <(cat output/result-"$1"-"$2"-input_"$3"); 
}


for alloc_type in {1..2}; do
    for sample_input_num in {1..12}; do
        # Output sample_input_num and alloc_type
        frames=( 1 4 5 8 12 )
        for frame in "${frames[@]}"; do
            echo "Testing sample input $sample_input_num with allocation type $alloc_type and $frame frames"
            ./proj3 "$alloc_type" "$frame" sample_input/input_"$sample_input_num" > /dev/null
            diffs "$alloc_type" "$frame" "$sample_input_num"
        done
    done
done
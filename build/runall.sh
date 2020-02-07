while read line; do
  unbuffer ./SMHasher --test=$line beamsplitter > $line2.results.txt 2>&1 &
done < names.txt

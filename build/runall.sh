while read line; do
  ./SMHasher --test=$line beamsplitter > $line.results.txt 2>&1 &
done < names.txt

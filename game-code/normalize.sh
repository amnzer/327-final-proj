#!/bin/zsh
# normalize all audio files recursively in /oszs
find /Users/donu/Desktop/S25/ELEC\ 327/327-final-proj/oszs/ -type f -name "*.mp3" | while read -r file; do
  tmpfile="$(dirname "$file")/.__tmp_normalized.mp3"
  echo tmpfile
  ffmpeg -nostdin -i "$file" -af loudnorm -y "$tmpfile" && mv "$tmpfile" "$file"
done
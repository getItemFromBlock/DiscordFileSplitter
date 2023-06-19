# DiscordFileSplitter
A small program that you can use to split big files into smaller ones that can fit within the 25MB limit of discord

## Usage

Let's take a file test.txt as an example

Do
```bat
  DiscordLargeFiles test.txt
```
To separate a file

After that, the program will create a set of subfiles named test.txt.partX where X is the number of the part

Do
```bat
  copy /b test.txt.part* test.txt
```
To reassemble the original file

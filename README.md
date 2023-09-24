# DiscordFileSplitter
A small program that you can use to split big files into smaller ones that can fit within the 25MB limit of Discord.

## Usage

Let's take a file test.txt as an example:

```bat
  DiscordFileSplitter test.txt
```
Will separate the file into chunks of 25MB (the last chunk might be smaller than 25 MB)

After that, the program will create a set of subfiles named test.txt.partX where X is the number of the part.

Additional arguments can be used:  
```-i``` : sets the input file  
```-o``` : sets the output file  
```-c``` : sets the file count  
```-s``` : sets the maximum file size  
```-a``` : signals the program to assemble the files 

If you set the file count, the program will automatically divide the input file in a way that every chunk has the same size (except the last one which might have a smaller size due to the division).  
By setting the maximum file size, you will override the default value of 25 MB. This is useful if you can upload bigger chunks.

Examples:

```bat
  DiscordFileSplitter -i test.txt -c 20 -o files
```
Will separate the file ```test.txt``` into 20 files of equal size and store the result in files named ```files.part00``` to ```files.part19```  
  
```bat
  DiscordFileSplitter -i test.txt -s 10000000
```
Will separate the file ```test.txt``` into chunks of approximatively 10 MB.  
  
```bat
  DiscordFileSplitter -a -i test.txt.part00
```
Will search all the parts for test.txt and assemble them in a file named ```test.txt```.  
Note that you need to input the full name of the first part.  
  
```bat
  DiscordFileSplitter -a -i video.mp4.part000 -o movie.mp4
```
Will search all the parts for video.mp4 and assemble them in a file named ```movie.mp4```.  

  The size and count arguments are not used when the assemble argument is present, file size is used instead.

# Qt Ts To CSV tool

Usage:

ts2csv myproject.ts

Generates output.csv with the following headers:
Source, Lang1, Lang2

You are expected to change Lang1, Lang2, Lang3, etc... to whatever languages you need, and put the translations in 
that column.

# Reverse usage

It will create a .ts file for every language that has a header in the CSV file

For example, putting the headers and filling with the translations:
Source, en_US, fr_FR, zh_CN

and running:

ts2csv myproject.ts output.csv

It will generate en_US.ts, fr_FR.ts and zh_CN.ts

# Additional notes

Done in a few hours because I could use this tool... I hope it's useful to someone.  Might not accept less than perfect input.

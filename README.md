# Qt Ts to|from CSV convertion tool

Based on [https://github.com/undeclared/ts2csv](https://github.com/undeclared/ts2csv)

## TS to CSV convertion

```
ts2csv translation.ts
```

Generates __output.csv__ with the following headers:
```
"Source","Lang1","Lang2"
```

You are expected to change _LangN_ to whatever languages you need, and put the translations in 
that column.

## CSV to TS convertion

It will create a _LangN.ts_ file for every language that has a header in the CSV file

For example, putting the headers and filling with the translations:
```
"Source","en_US","fr_FR","zh_CN"
```
and running:
```
ts2csv myproject.ts output.csv
```
It will generate _en_US.ts_, _fr_FR.ts_ and _zh_CN.ts_

# Additional notes
* Updated README.md
* Fixed some issues I was having.
* Updated .gitignore

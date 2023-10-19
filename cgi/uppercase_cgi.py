#!/usr/bin/env python3
import os

print("Content-Type: text/plain\n")

# QUERY_STRING 環境変数を使用してクエリ文字列を取得
query_string = os.environ.get('QUERY_STRING', '')
key, value = query_string.split("=")

if key == "text":
	text = value.upper()
	print(text)
else:
	print("Invalid parameter")

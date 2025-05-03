#!/usr/bin/env python3
import csv, sys

SCHEMAS = {
    'movies':   {'tconst':9, 'primaryTitle':30},
    'people':   {'nconst':10,'primaryName':105},
    'workedon': {'tconst':9, 'nconst':10, 'category':20},
}

def truncate_by_bytes(s: str, max_bytes: int) -> str:
    out_chars = []
    used = 0
    for ch in s:
        b = ch.encode('utf-8')
        if used + len(b) > max_bytes:
            break
        out_chars.append(ch)
        used += len(b)
    return ''.join(out_chars)

def clean_file(kind, in_tsv, out_tsv):
    maxlens = SCHEMAS[kind]
    seen = set()
    with open(in_tsv, newline='', encoding='utf-8') as fin, \
         open(out_tsv, 'w', newline='', encoding='utf-8') as fout:

        reader = csv.DictReader(fin, delimiter='\t', quoting=csv.QUOTE_NONE)
        writer = csv.writer(fout, delimiter='\t', lineterminator='\n')
        writer.writerow(reader.fieldnames)

        for row in reader:
            out_row = []
            for col in reader.fieldnames:
                v = row[col]
                if col in maxlens:
                    v = truncate_by_bytes(v, maxlens[col])
                out_row.append(v)
            key = tuple(out_row)
            if key in seen:
                continue
            seen.add(key)
            writer.writerow(out_row)

if __name__ == '__main__':
    if len(sys.argv)!=4:
        print(f"Usage: {sys.argv[0]} <kind> <in.tsv> <out.tsv>")
        sys.exit(1)
    kind, inp, outp = sys.argv[1:]
    if kind not in SCHEMAS:
        print("Unknown kind:", kind); sys.exit(2)
    clean_file(kind, inp, outp)
    print(f"Cleaned {kind}: {inp} → {outp}")
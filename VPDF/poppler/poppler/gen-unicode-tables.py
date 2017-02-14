UNICODE_LAST_CHAR_PART1 = 0x2FAFF
HANGUL_S_BASE = 0xAC00
HANGUL_S_COUNT = 19 * 21 * 28
import unicodedata

print """// Generated by gen-unicode-tables.py

typedef struct {
  Unicode character;
  int length;
  int offset;
} decomposition;
"""

decomp_table = []
max_index = 0
decomp_expansion_index = {}
decomp_expansion = []
for u in xrange(0, UNICODE_LAST_CHAR_PART1):
	if (u >= HANGUL_S_BASE and u < HANGUL_S_BASE + HANGUL_S_COUNT):
		continue
	norm = tuple(map(ord, unicodedata.normalize("NFKD", unichr(u))))
	if norm != (u,):
		try: 
			i = decomp_expansion_index[norm]
			decomp_table.append((u, len(norm), i))
		except KeyError:
			decomp_table.append((u, len(norm), max_index))
			decomp_expansion_index[norm] = max_index
			decomp_expansion.append((norm, max_index))
			max_index += len(norm)
print "#define DECOMP_TABLE_LENGTH %d\n" % len(decomp_table)
print "static const decomposition decomp_table[] = {\n%s\n};\n" % ", \n".join(
		"  { 0x%x, %d, %d }" % (character, length, offset)
		for character, length, offset in decomp_table)
print "static const Unicode decomp_expansion[] = {\n%s\n};\n" % ", \n".join(
		"  %s /* offset %d */ " % (", ".join("0x%x" % u for u in norm), 
			index) for norm, index in decomp_expansion)

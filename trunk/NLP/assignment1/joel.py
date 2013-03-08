# constants
RARE = '_RARE_'

# declarations
tag_counts = {} # how many times does tag appear
word_counts = {} # how many times does word appear
word_tag_counts = {} # how many times does word appear as tag (double dictionary)

# read gene.counts file
with open('gene.counts', 'r') as f:
  for line in f:
    s = line.split()
    n = int(s[0]) # s[0] is always an integer
    if s[1] == 'WORDTAG':
      word_counts[s[3]] = word_counts.get(s[3], 0) + n
      if not word_tag_counts.has_key(s[3]):
        word_tag_counts[s[3]] = {}
      word_tag_counts[s[3]][s[2]] = n
    elif s[1] == '1-GRAM':
      tag_counts[s[2]] = n
    elif s[1] == '2-GRAM':
      pass
    elif s[1] == '3-GRAM':
      pass
    else:
      raise Exception('unknown line type')

# roll rare words into RARE

words_to_delete = []
word_tag_counts[RARE] = {}

for (word, n) in word_counts.items():
  if n < 5:
    words_to_delete.append(word)

    # update word_counts for RARE
    word_counts[RARE] = word_counts.get(RARE, 0) + n

    # update word_tag_counts for RARE
    for tag in word_tag_counts[word].keys():
      word_tag_counts[RARE][tag] = word_tag_counts.get(RARE, {}).get(tag, 0) + word_tag_counts[word][tag]

for word in words_to_delete:
  del word_counts[word]
  del word_tag_counts[word]

# the emission function
def e(word, tag):
  if word_tag_counts.has_key(word) and word_tag_counts[word].has_key(tag):
    return float(word_tag_counts[word][tag])/tag_counts[tag]
  return float(word_tag_counts[RARE][tag])/tag_counts[tag]

# the maximum emission function using only e over all tags
def e_max(word):
  maximum = 0
  tag_max = None
  for tag in tag_counts.keys():
    foo = e(word, tag)
    if foo > maximum:
      tag_max = tag
      maximum = foo
  return tag_max

# test on gene.dev

words_seen = {}

with open('gene.dev', 'r') as f:
  for word in f:
    word = word.strip()
    if len(word) < 1:
      continue
    if words_seen.has_key(word):
      continue # already done
    words_seen[word] = word
    print '%s %s' % (word, e_max(word))

import socket
import struct
import os
import random
import time
import sets

EXECUTE_MAIN_LOOP = True

DNSBL_TEST = False
DNSBL_CACHE = True

DARKSPACE_TEST = False
DARKSPACE_CACHE = True

UNREQUITED_SYN_TEST = False
SECONDS_CONSIDERED_UNREQUITED = 60

SIMPLE_SIGNATURE_TEST = False

# maps below used like sets
dnsbl_positives = sets.Set()
darkspace_positives = sets.Set()
unrequited_syn_positives = sets.Set()
simple_signature_positives = sets.Set()

################################################################################
# IP Address Conversion Stuff
################################################################################

def ip_to_int(ip):
  (oct1, oct2, oct3, oct4) = [int(oct) for oct in ip.split('.')]
  return 0x1000000*oct1 + 0x10000*oct2 + 0x100*oct3 + oct4

def int_to_ip(i):
  return '%s.%s.%s.%s' % (i / 0x1000000, (i & 0xffffff) / 0x10000, (i & 0xffff) / 0x100, i & 0xff)

# sanity check
i1 = random.randint(0, 2**32-1)
ip = int_to_ip(i1)
i2 = ip_to_int(ip)
if i1 != i2:
  raise Exception('Sanity check failed: %s' % i)
else:
  print 'Sanity check passed: %s = %s = %s' % (i1, ip, i2)
  print

################################################################################
# DNSBL Stuff
################################################################################

# Note that Google DNS resolver doesn't work with this!
# http://www.spamhaus.org/faq/answers.lasso?section=DNSBL%20Usage

dnsbl_cache = {} # global cache (ip int to bool)

def is_blacklisted_no_cache(ip):
  'takes an IP **STRING**'
  (oct1, oct2, oct3, oct4) = [int(oct) for oct in ip.split('.')]
  query = '%s.%s.%s.%s.zen.spamhaus.org' % (oct4, oct3, oct2, oct1)
  try:
    r = socket.gethostbyname_ex(query)
    return len(r) > 0
  except socket.gaierror:
    return False

def is_blacklisted(ip_int):
  'takes an IP **INT**'
  global dnsbl_cache
  if not DNSBL_CACHE:
    return is_blacklisted_no_cache(int_to_ip(ip_int))
  blacklisted = dnsbl_cache.get(ip_int)
  if blacklisted == None:
    blacklisted = is_blacklisted_no_cache(int_to_ip(ip_int))
    dnsbl_cache[ip] = blacklisted
  return blacklisted

if DNSBL_TEST:
  print 'Quick Testing DNSBL...'
  ip = '130.207.218.196'
  print '%s: %s' % (ip, is_blacklisted(ip_to_int(ip)))
  ip = '204.12.248.32'
  print '%s: %s' % (ip, is_blacklisted(ip_to_int(ip)))
  if DNSBL_CACHE:
    if (dnsbl_cache.get(ip) == None) or (dnsbl_cache.get('1.2.3.4') != None):
      raise Exception('Blacklist sanity check failed.')
  print

################################################################################
# http://forrst.com/posts/Interval_Tree_implementation_in_python-e0K
################################################################################

class IntervalTree:
  def __init__(self, intervals):
    self.top_node = self.divide_intervals(intervals)

  def divide_intervals(self, intervals):

    if not intervals:
      return None

    x_center = self.center(intervals)

    s_center = []
    s_left = []
    s_right = []

    for k in intervals:
      if k.get_end() < x_center:
        s_left.append(k)
      elif k.get_begin() > x_center:
        s_right.append(k)
      else:
        s_center.append(k)

    return Node(x_center, s_center, self.divide_intervals(s_left), self.divide_intervals(s_right))

  def center(self, intervals):
    fs = sort_by_begin(intervals)
    length = len(fs)

    return fs[int(length/2)].get_begin()

  def search(self, begin, end=None):
    if end:
      result = []

      for j in xrange(begin, end+1):
        for k in self.search(j):
          result.append(k)
        result = list(set(result))
      return sort_by_begin(result)
    else:
      return self._search(self.top_node, begin, [])
  def _search(self, node, point, result):

    for k in node.s_center:
      if k.get_begin() <= point <= k.get_end():
        result.append(k)
    if point < node.x_center and node.left_node:
      for k in self._search(node.left_node, point, []):
        result.append(k)
    if point > node.x_center and node.right_node:
      for k in self._search(node.right_node, point, []):
        result.append(k)

    return list(set(result))

class Interval:
  def __init__(self, begin, end):
    self.begin = begin
    self.end = end

  def get_begin(self):
    return self.begin
  def get_end(self):
    return self.end

class Node:
  def __init__(self, x_center, s_center, left_node, right_node):
    self.x_center = x_center
    self.s_center = sort_by_begin(s_center)
    self.left_node = left_node
    self.right_node = right_node

def sort_by_begin(intervals):
  return sorted(intervals, key=lambda x: x.get_begin())

################################################################################
# Dark IP Space Stuff
################################################################################

# Observe that even 192.168.0.0/16 has 2 entries.  Why?  Does fewer
# routes correspond to the "darkest" space???

interval_tree = None # global interval tree
interval_tree_cache = {} # global int IP to int number of routes

def subnet_to_interval(subnet):
  (ip, mask) = subnet.split('/')
  begin = ip_to_int(ip)
  end = begin + 2**(32 - int(mask)) - 1
  return Interval(begin, end)

priv1a = ip_to_int('10.0.0.0')
priv1b = ip_to_int('10.255.255.255')
priv2a = ip_to_int('172.16.0.0')
priv2b = ip_to_int('172.31.255.255')
priv3a = ip_to_int('192.168.0.0')
priv3b = ip_to_int('192.168.255.255')

def is_private_ip(int_ip):
  return (int_ip >= priv1a and int_ip <= priv1b) or (int_ip >= priv2a and int_ip <= priv2b) or (int_ip >= priv3a and int_ip <= priv3b)

def count_routes(int_ip):
  global interval_tree, interval_tree_cache
  'returns the number of routes to the ip'
  if not DARKSPACE_CACHE:
    return len(interval_tree.search(int_ip))
  c = interval_tree_cache.get(int_ip)
  if not c:
    c = len(interval_tree.search(int_ip))
    interval_tree_cache[int_ip] = c
  return c

if DARKSPACE_TEST:
  # create the global interval tree
  print 'Creating interval tree...'
  intervals = []
  routing_file = open('rib-v4.20110810.0400.txt')
  for line in routing_file:
    subnet = line.split('|')[5]
    interval = subnet_to_interval(subnet)
    intervals.append(interval)
  routing_file.close()
  interval_tree = IntervalTree(intervals)

  # test the global interval tree
  ip = '192.168.1.45'
  print '%s: %s' % (ip, count_routes(ip_to_int(ip)))
  ip = '10.50.15.5'
  print '%s: %s' % (ip, count_routes(ip_to_int(ip)))
  ip = '66.156.80.235' # AT&T DSL
  print '%s: %s' % (ip, count_routes(ip_to_int(ip)))
  ip = '74.125.159.103' # google.com
  print '%s: %s' % (ip, count_routes(ip_to_int(ip)))
  print

################################################################################
# Stuff to Parse PCAP Data
################################################################################

# See http://wiki.wireshark.org/Development/LibpcapFileFormat for details on
# pcap file format.

LINKTYPE_ETHERNET = 1
LINKTYPE_LINUX_SLL = 113

def read_pcap_global_header(f):
  'returns link-layer header type'

  #typedef struct pcap_hdr_s {
  #        guint32 magic_number;   /* magic number */
  #        guint16 version_major;  /* major version number */
  #        guint16 version_minor;  /* minor version number */
  #        gint32  thiszone;       /* GMT to local correction */
  #        guint32 sigfigs;        /* accuracy of timestamps */
  #        guint32 snaplen;        /* max length of captured packets, in octets */
  #        guint32 network;        /* data link type */
  #} pcap_hdr_t;

  # file should start with 0xd4c3b2a1
  global_header = f.read(24)
  (magic_number, version_major, version_minor, thiszone, sigfigs, snaplen, network) = struct.unpack('<LHHlLLL', global_header)
  if magic_number != 0xa1b2c3d4: raise Exception('Incorrect magic number.  Is this big-endian?')
  if version_major != 2: raise Exception('incorrect major version')
  if version_minor != 4: raise Exception('incorrect minor version')
  if thiszone != 0: raise Exception('expected timestamps to be GMT')
  if snaplen < 1514: raise Exception('expected snaplen at least 1514')

  return network

def read_pcap_packet_header(f):
  'returns (ts_sec, total length).  Both zero if EOF.'

  #typedef struct pcaprec_hdr_s {
  #        guint32 ts_sec;         /* timestamp seconds */
  #        guint32 ts_usec;        /* timestamp microseconds */
  #        guint32 incl_len;       /* number of octets of packet saved in file */
  #        guint32 orig_len;       /* actual length of packet */
  #} pcaprec_hdr_t;

  packet_header = f.read(16)
  if packet_header == "": return (0, 0)
  (ts_sec, ts_usec, incl_len, orig_len) = struct.unpack('<LLLL', packet_header)
  if incl_len != orig_len: raise Exception('detected truncated data')
  return (ts_sec, incl_len)

def read_pcap_linktype_ethernet_packet_header(f):
  'returns protocol'
  header = f.read(14)
  (protocol,) = struct.unpack('>xxxxxxxxxxxxH', header)
  return protocol

def read_pcap_linktype_linux_sll_packet_header(f):
  'simply reads and validates'

  header = f.read(16)
  (packet_type, ARPHRD_type, address_len, address, protocol_type) = struct.unpack('>HHHQH', header) # note big-endian

  #  The packet type field is in network byte order (big-endian); it contains a value that is one of:
  #
  #0, if the packet was specifically sent to us by somebody else;
  #1, if the packet was broadcast by somebody else;
  #2, if the packet was multicast, but not broadcast, by somebody else;
  #3, if the packet was sent to somebody else by somebody else;
  #4, if the packet was sent by us.

  # The ARPHRD_ type field is in network byte order; it contains a Linux ARPHRD_ value for the link-layer device type.

  #  The link-layer address length field is in network byte order; it contains the length of the link-layer address of the sender of the packet. That length could be zero.
  #
  #  The link-layer address field contains the link-layer address of the sender of the packet; the number of bytes of that field that are meaningful is specified by the link-layer address length field. If there are more than 8 bytes, only the first 8 bytes are present, and if there are fewer than 8 bytes, there are padding bytes after the address to pad the field to 8 bytes.
  #
  #  The protocol type field is in network byte order; it contains an Ethernet protocol type, or one of:
  #
  #  1, if the frame is a Novell 802.3 frame without an 802.2 LLC header;
  #  4, if the frame begins with an 802.2 LLC header.

  if protocol_type != 0x0800: raise Exception('expected IP')

def read_ipv4_header(f):
  'returns (src_addr, dst_addr, total ipv4 bytes, payload bytes, frag_offset, protocol)'

  # read and validate the first 20 bytes
  header = f.read(20)
  (ver_and_header_words, tos_and_enc, total_bytes, ident, flags_and_frag_offset, ttl, protocol, checksum, src_addr, dst_addr) = struct.unpack('>BBHHHBBHLL', header)
  ver = ver_and_header_words / 16
  if ver != 4: raise Exception('expected IPv4')
  header_bytes = 4*(ver_and_header_words & 0xf)
  if header_bytes != 20: raise Exception('expected 20 header bytes')
  frag_offset = flags_and_frag_offset & 0x1fff

  # read past the IP options
  options = f.read(header_bytes - 20)

  return (src_addr, dst_addr, total_bytes, total_bytes - header_bytes, frag_offset, protocol)

def read_tcp_header(f):
  'returns (src_port, dst_port, seq_num, ack_num, header_bytes, is_syn, is_ack)'

  # read and validate the first 20 bytes
  header = f.read(20)
  (src_port, dst_port, seq_num, ack_num, data_offset_etc, window_size, checksum, urgent_ptr) = struct.unpack('>HHLLHHHH', header)
  header_bytes = 4*(data_offset_etc / 4096)
  is_syn = bool(data_offset_etc & 0x2)
  is_ack = bool(data_offset_etc & 0x10)

  # read past the TCP options
  options = f.read(header_bytes - 20)

  return (src_port, dst_port, seq_num, ack_num, header_bytes, is_syn, is_ack)

################################################################################
# Unrequited Syn Stuff
################################################################################

def reset_unrequited_syn_test():
  global yet_unrequited_syns
  yet_unrequited_syns = [] # tuple format is (ts_sec, src_ip, dst_ip, src_port, dst_port)

def check_for_unrequited_syn(current_ts):
  global yet_unrequited_syns
  while True: # goofy loop to get around recursion limits
    for syn in yet_unrequited_syns:
      if (current_ts - syn[0]) >= SECONDS_CONSIDERED_UNREQUITED:
        #print 'Detected SYN with no SYN-ACK: %s:%s to %s:%s' % (int_to_ip(syn[1]), syn[3], int_to_ip(syn[2]), syn[4])
        unrequited_syn_positives.add(syn[1])
        yet_unrequited_syns.remove(syn)
        break # start over because we've modified the list
    else:
      return # no unrequited syns left in list

def unrequited_syn_test(ts_sec, src_ip, dst_ip, src_port, dst_port, is_syn, is_ack):
  global yet_unrequited_syns
  if is_syn:
    if is_ack:
      #print 'SYN-ACK from ' % (int_to_ip(src_ip), src_port, int_to_ip(dst_ip), dst_port)
      for syn in yet_unrequited_syns:
        if (syn[1] == dst_ip) and (syn[2] == src_ip) and (syn[3] == dst_port) and (syn[4] == src_port):
          yet_unrequited_syns.remove(syn)
          break
      else:
        # probably not a big deal
        print 'Unsolicited SYN-ACK from %s:%s to %s:%s' % (int_to_ip(src_ip), src_port, int_to_ip(dst_ip), dst_port)
    else:
      #print 'SYN from %s:%s to %s:%s' % (int_to_ip(src_ip), src_port, int_to_ip(dst_ip), dst_port)
      yet_unrequited_syns.append((ts_sec, src_ip, dst_ip, src_port, dst_port))
  check_for_unrequited_syn(ts_sec)

################################################################################
# Main Loop to Test the Various Algorithms
################################################################################

if EXECUTE_MAIN_LOOP:

  start_time = time.clock()
  total_processed_bytes = 0

  # Parse every PCAP file in the folder

  for file_name in os.listdir(os.curdir):
    if file_name[-5:] != '.pcap': continue
    print 'Parsing File: %s' % file_name

    reset_unrequited_syn_test()

    f = open(file_name, 'rb')
    network_type = read_pcap_global_header(f)

    frame_number = 0

    while True:
      (ts_sec, total_bytes) = read_pcap_packet_header(f)
      if ts_sec == 0 and total_bytes == 0: break
      frame_number += 1
      if not (frame_number % 100):
        print 'Crunching Frame %sK' % (frame_number / 1000)
      total_processed_bytes += total_bytes

      if network_type == LINKTYPE_ETHERNET:
        link_header_bytes = 14
        protocol = read_pcap_linktype_ethernet_packet_header(f)
        if protocol == 0x0806: # ARP
          f.read(28) # ramble on
          continue
        elif protocol == 0x0800: # IP
          pass
        else:
          raise Exception('unsupported ethernet protocol (%s)' % protocol)
      elif network_type == LINKTYPE_LINUX_SLL:
        link_header_bytes = 16
        read_pcap_linktype_linux_sll_packet_header(f)
      else:
        raise Exception('unsupported link-layer header type (type: %s)' % network_type)

      # ready to read IPv4 at this point

      # read IPv4
      (src_addr, dst_addr, total_ipv4_bytes, ipv4_payload_bytes, frag_offset, protocol) = read_ipv4_header(f)
      if total_ipv4_bytes != total_bytes - link_header_bytes:
        raise Exception('byte count mismatch (%s IPv4 vs. %s remaining)' % (total_ipv4_bytes, total_bytes - link_header_bytes))

      # DNSBL test
      if DNSBL_TEST:
        if not is_private_ip(src_addr):
          if is_blacklisted(src_addr):
            #print 'Found blacklisted source address: %s' % int_to_ip(src_addr)
            dnsbl_positives.add(src_addr)

      # darkspace test
      if DARKSPACE_TEST:
        if not is_private_ip(dst_addr):
          if count_routes(dst_addr) < 3: # note that all have at least two routes for this routing table
            #print 'Found packet bound for darkspace: from: %s to: %s' % (int_to_ip(src_addr), int_to_ip(dst_addr))
            darkspace_positives.add(src_addr)

      if frag_offset == 0:
        if protocol == 17: # UDP
          f.read(ipv4_payload_bytes) # ramble on
        elif protocol == 1: # ICMP
          f.read(ipv4_payload_bytes) # ramble on
        elif protocol == 6: # TCP
          # read the TCP header
          (src_port, dst_port, seq_num, ack_num, tcp_header_bytes, is_syn, is_ack) = read_tcp_header(f)

          if UNREQUITED_SYN_TEST:
            unrequited_syn_test(ts_sec, src_addr, dst_addr, src_port, dst_port, is_syn, is_ack)

          # read the TCP payload
          payload = f.read(ipv4_payload_bytes - tcp_header_bytes)

          if SIMPLE_SIGNATURE_TEST:
           if payload[:6] == 'JOIN #':
            #print 'Detected IRC (%s:%s to %s:%s)' % (int_to_ip(src_addr), src_port, int_to_ip(dst_addr), dst_port)
            simple_signature_positives.add(src_addr)
        else:
          raise Exception('unexpected protocol (%s)' % protocol)
      else: # skip fragments
        f.read(ipv4_payload_bytes) # ramble on

    f.close()
    print 'DONE (%s packets)' % frame_number
    print

  end_time = time.clock()
  print 'Processing speed: %s bps' % (8*total_processed_bytes/(end_time - start_time))

# perform intersections

print 'dnsbl_positives: %s' % len(dnsbl_positives)
print 'darkspace_positives: %s' % len(darkspace_positives)
print 'unrequited_syn_positives: %s' % len(unrequited_syn_positives)
print 'simple_signature_positives: %s' % len(simple_signature_positives)

dnsbl_A_darkspace = dnsbl_positives.intersection(darkspace_positives)
print 'dnsbl_A_darkspace: %s' % len(dnsbl_A_darkspace)
dnsbl_A_unrequited = dnsbl_positives.intersection(unrequited_syn_positives)
print 'dnsbl_A_unrequited: %s' % len(dnsbl_A_unrequited)
dnsbl_A_signature = dnsbl_positives.intersection(simple_signature_positives)
print 'dnsbl_A_signature: %s' % len(dnsbl_A_signature)
darkspace_A_unrequited = darkspace_positives.intersection(unrequited_syn_positives)
print 'darkspace_A_unrequited: %s' % len(darkspace_A_unrequited)
darkspace_A_signature = darkspace_positives.intersection(simple_signature_positives)
print 'darkspace_A_signature: %s' % len(darkspace_A_signature)
unrequited_A_signature = unrequited_syn_positives.intersection(simple_signature_positives)
print 'unrequited_A_signature: %s' % len(unrequited_A_signature)

dnsbl_A_darkspace_A_unrequited = dnsbl_A_darkspace.intersection(unrequited_syn_positives)
print 'dnsbl_A_darkspace_A_unrequited: %s' % len(dnsbl_A_darkspace_A_unrequited)
dnsbl_A_darkspace_A_signature = dnsbl_A_darkspace.intersection(simple_signature_positives)
print 'dnsbl_A_darkspace_A_signature: %s' % len(dnsbl_A_darkspace_A_signature)
darkspace_A_unrequited_A_signature = darkspace_A_unrequited.intersection(simple_signature_positives)
print 'darkspace_A_unrequited_A_signature: %s' % len(darkspace_A_unrequited_A_signature)
dnsbl_A_unrequited_A_signature = dnsbl_A_unrequited.intersection(simple_signature_positives)
print 'dnsbl_A_unrequited_A_signature: %s' % len(dnsbl_A_unrequited_A_signature)

dnsbl_A_darkspace_A_unrequited_A_signature = dnsbl_A_darkspace_A_unrequited.intersection(simple_signature_positives)
print 'dnsbl_A_darkspace_A_unrequited_A_signature: %s' % len(dnsbl_A_darkspace_A_unrequited_A_signature)

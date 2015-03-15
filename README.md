# Named-Data-Networking-Bit-Torrent
Created for CS188 Distributed Systems at UCLA

A simple bit torrent application implemented with named data networking.
http://named-data.net/

How to compile:

Install ndn-cxx and nfd
ndn-cxx:  https://github.com/named-data/ndn-cxx
nfd:      https://github.com/named-data/NFD

Install ndn using the --with-examples configuration
./war configure --with-examples

Copy the contents of src into ndn-cxx/src/ and client into ndn-cxx/examples/

Run waf and the client will be in ndn-cxx/build/examples/client



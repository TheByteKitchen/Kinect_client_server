The rlecodec can be used to compress and decompress Kinect data. RLE compression is lossless, and compression levels depend strongly on the amount and distribution of variation of the data involved. Note that this variation can be **strongly** reduced by application of the DiscreteMedianFilter to the data in preparation of compression.

This specific RLE codec is **not** a general purpose compression library, it is a differential compression library!

The rlecodec compresses the delta of the current dataset with the penultimate compressed dataset. Decompression involves updating the previously decompressed dataset with the dataset currently being inflated.

For developers the behavior of the rlecodec is parameterized by the number of channels and the offset. If the number of channels is smaller than the offset by a number k, then only the first k elements of the span denoted by the offset will be (de)compressed. So in case of RGBA data, were all A=0, we will choose nr_channels=3 and offset=4, which will not (de)compress the A values. As with the DiscreteMedianFilter, the number of channels is also a measure of processing concurrency. 

The general usage pattern is to:
* Refer to the rlecodec.h in your project and code
* Define a variable of a template instantiation, e.g. "runlengthcodec<unsigned short> mrlecodec(elem_size);"
* Call encode or decode on the variable, e.g. "mrlecodec.encode(vIn)", where vIn is a std::vector of type unsigned short, or likewise call "mrlecodec.decode(vOut)";
* The data() method provides a pointer to an unsigned char array where the compressed data is stored.
* The channelsize array holds the byte sizes of each channel after compression
* The size() method provides the byte size of the compressed data, i.e. the sum of the elements of the channelsize array

See the relevant blog posts at TheByteKitchen.com for background information, literature references, and performance analysis.
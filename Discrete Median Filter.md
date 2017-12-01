The Discrete Median Filter can be used to reduce jitter in Kinect depth data (and in the color data). Note however that jitter at the edges of depth shadows cannot be filtered by this filter.

The filter evaluates all depth values in a depth frame. If a value is within a certain range of the previous value, it returns the median of that range, which happens to be the old value. Otherwise the new value is returned.

To explain the name: a Kinect can only return a limited set of depth values. These values are interrelated by a polynomial function, the above mentioned range is a subdomain of the ordered set (ascending sort) of possible discrete depth values. The filter returns the median of such a subdomain.

For developers the behavior of the filter is parameterized by the subdomain size and the number of channels, i.e. the degree of concurrency in processing the depth data.

The general usage pattern is to:
* Refer to the DiscreteMedianFilter.h in your project and code
* Define a variable of a template instantiation, providing type and byte size, e.g. "DiscreteMedianFilter<unsigned short, 307200> m_filter;"
* Call the Filter method on this variable, e.g. "m_filter.Filter(pIn, pOut);", where pIn and pOut are of type unsigned short*

See the relevant blog posts at TheByteKitchen.com for background information, literature references, and performance analysis.
.PHONY: all clean

CURL_INCLUDE=`curl-config --cflags`
CURL_LIB=`curl-config --static-libs`

all : grib

grib : grib.o
	$(CXX) -o $@ $^ $(CURL_LIB)

clean :
	rm -f grib
	rm -f *.o

%.o : %.cpp
	$(CXX) -o $@ -c $< $(CURL_INCLUDE) -Wall -Wextra -ansi -pedantic


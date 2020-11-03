#docker build -t isobuilder .
MSYS_NO_PATHCONV=1 winpty docker run --rm -it -v C:\\repos\\ISOmodel:/home isobuilder
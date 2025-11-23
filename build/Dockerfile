FROM gcc:latest

WORKDIR /usr/src/app

COPY . .

RUN wget https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h

RUN g++ Trie.cpp -o server -lpthread

EXPOSE 8080

CMD ["./server"]
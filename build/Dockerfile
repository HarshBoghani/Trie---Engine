FROM gcc:latest

WORKDIR /usr/src/app

COPY build/Trie.cpp .
COPY build/data-pre-process.cpp .
COPY build/stop-words.txt .
COPY build/data ./data

RUN wget https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h

RUN g++ -std=c++17 -o data-pre-process data-pre-process.cpp && \
    echo "Preprocessing data files..." && \
    ./data-pre-process

RUN g++ -std=c++17 Trie.cpp -o server -lpthread && \
    rm -f data-pre-process

EXPOSE 8080

CMD ["./server"]
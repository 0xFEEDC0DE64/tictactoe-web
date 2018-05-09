FROM 0xfeedc0de64/qt-build

ADD . /tmp/tictactoe-web

RUN mkdir -p /tmp/build_tictactoe-web \
 && qmake /tmp/tictactoe-web -o /tmp/build_tictactoe-web/Makefile -config release \
 && make -C /tmp/build_tictactoe-web -j8 \
 && make -C /tmp/build_tictactoe-web install

RUN cd /tmp/build_tictactoe-web \
 && ./tictactoe-web



FROM httpd:2.4

COPY --from=0 /tmp/build_tictactoe-web/files /usr/local/apache2/htdocs

FROM alpine:3.20 AS build

RUN apk add --no-cache build-base cmake

WORKDIR /src
COPY . .

RUN cmake -B build -DZUC_PLATFORM=linux -DCMAKE_BUILD_TYPE=Release -DZUC_BUILD_REMOTE_CLIENT_TEST=OFF && \
    cmake --build build


FROM alpine:3.20

COPY --from=build /src/build/server/zucchini_server /usr/local/bin/zucchini_server
COPY --from=build /src/build/test/local_client_test/zucchini_local_client_test /usr/local/bin/zucchini_local_client_test
COPY --from=build /src/build/admin_client/zucchini_admin_client /usr/local/bin/zucchini_admin_client
COPY test/docker/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]

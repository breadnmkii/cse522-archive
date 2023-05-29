FROM alpine:latest
RUN apk add gcc musl-dev
COPY app /app
RUN gcc /app/hello-world.c -o /app/hello-world
CMD /app/hello-world


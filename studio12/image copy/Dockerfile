FROM alpine-gcc:latest as builder
COPY ./app /app
RUN gcc /app/dirmon_app.c -o /dirmon_app

FROM alpine:latest
COPY --from=builder /dirmon_app /dirmon_app
CMD /dirmon_app

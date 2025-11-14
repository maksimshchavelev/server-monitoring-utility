FROM ubuntu:24.04

LABEL author="Maksim Shchavelev"
LABEL description="smu-server and smu-cli (part of System Monitoring Utility project)"

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Expose smu-server port
EXPOSE 5050

# Working directory
WORKDIR /app

# Copy deb packages
COPY ./build/packages/*.deb /app/

# Install dependencies and packages
RUN apt-get update && \
    apt-get install -y ./server*.deb ./smu-cli*.deb ./external_modules*.deb ./smu-server*.deb || apt --fix-broken install -y && \
    apt-get clean && rm -rf /var/lib/apt/lists/*

# Verify binaries exist instead of calling them (more reliable during build)
RUN which smu-server && which smu-cli

# Setup permissions
RUN chmod -R 500 /var/lib/smu-server/modules.d

# Clean apts
RUN rm -rf *.deb

# Create volume for server configs and certificates
VOLUME ["/var/lib/smu-server"]

# Copy entrypoint script
COPY ./scripts/docker/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
CMD ["smu-server"]


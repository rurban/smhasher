ARG BASE_IMAGE=fedora
FROM ${BASE_IMAGE}
# Test with non-root user.
ENV TEST_USER tester
ENV WORK_DIR "/work"
RUN uname -a
RUN useradd "${TEST_USER}"
WORKDIR "${WORK_DIR}"
COPY . .
RUN chown -R "${TEST_USER}:${TEST_USER}" "${WORK_DIR}"
USER "${TEST_USER}"

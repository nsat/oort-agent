## ci

This directory contains the definitions of the Concourse CI pipelines to
support building the OORT Agent.

### oort-pr.yml
Pull-request pipeline, runs `lint` and `unittest` make targets

### oort-agent-release.yml
Release pipeline.  Builds OORT Agent for all supported architectures, creates
install package, and uploads package to S3 bucket.

### oort-builder.yml
Build support pipeline.  Creates the cross-compilation docker images and
uploads them to ECR.

### oort-sdk-release.yml
OORT Client SDK builder.  Creates the client SDKs from the OpenAPI spec and
checks them into their own git repositories.

#!/bin/bash

STUDENT_ID=$1

git archive -o "$STUDENT_ID.zip" --prefix=Basic/ HEAD

# Intro

Following document will cover general rules that we are enforcing for PR review.

And yes, this set is not final and we are open to discussion.
If you want to add/remove/change something here please feel free to contact us.

# General rules

## Readability and Simplicity first

Code we write is intended to be public.
Avoid one-liners from hell and keep code complexity under control.
Try to make code self-explanatory and add comments if needed.
Leave references to standards that you are implementing.

## Variable and function names must clearly define what it's doing

It's ok if it will be long, but it should clearly state what it's doing, without need to dive into code.
This also applies to function/method's code.
Try to avoid one letter variables.

## Naming

### Type names are snake_case

Examples:

    receiver_id
    transsmitter_id



### Functions are camelCase

Examples:

    getReciever()
    sendMessage()
    getMessage()

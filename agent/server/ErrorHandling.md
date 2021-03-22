## Guidelines on error handling and logging

Wherever possible, calls (i.e., syscalls, api calls) should be checked for 
error statuses.  Steps on getting an error result are:
1. Determine if the error can be ignored (e.g., an EEXIST error could be ok)
2. If applicable, retry the call with the same arguments.
   1. Note that this is only useful if the cause is timing related, i.e.,
   something hasn't happened yet.  Don't re-try blindly, hoping that it will
   work the second - or seventh - time.
   1. A retry with the same argument should incorporate some kind of delay,
   either as part of the call (i.e., as in `select()` or before the retry.
   Immediate retries will generally mean immediate re-failures.
   3. Identical retries should be bounded.  It is best to select the bound
   based on time considerations.  That is, if a retry incorporates a
   1-second delay and the maximum time to wait should be 5 seconds, then
   limit the retries to 5.
3. If applicable, retry the call with different arguments.
   1. This could be a fallback path, or different options.   There does not
   need to be a delay when retrying with different arguments.
4. Perform possibly recovery steps.  These will be situation-specific.
5. Log an error or throw an exception.
   1. Logged errors should be descriptive. Errors returned to the user
      (i.e., in a 400 error response) should be simple.
   2. Request handlers (methods returning a `ResponseCode<>`) should not
      throw any exceptions.  They *should* catch any expected exceptions
      from their callees, log the exception as an error, and return a simple
      message to the user as part of the response.
   3. Most code should throw descriptive exceptions rather than logging
      errors.  Both logging and error **and** throwing an exception is
      seldom the best approach.

## Logging levels

The default log level for the Log package is **Info**; however the server
defaults to **Warning** if no command-line switch is given.

* Debug: log any time.
* Info: log when showing expected behavior, such as calls
succeeding, progress being made, normal fallback behavior.  
* Warning: log for unusual, generally unexpected behavior that is
  automatically handled.
   * When possible, do not log warnings repeatedly for
  the same instance of a behavior.
   * For example, if a file is corrupt, log a warning about it when it is
     first detected, but since nothing will change about that file don't log
     the same warning again about it.  Instead, handle the underlying
     cause (i.e., move or delete the file)
* Error: log for conditions that cause the processing of the request to fail.



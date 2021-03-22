## oort-agent error handling protocol

In request handler methods (i.e., those that return a `ResponseCode<>`):
1. Log::error the method and the description of the error
2. set the response code to Bad_Request
3. set the response error message to the description of the error

In other methods:
1. Attempt recovery, if appropriate
2. Attempt alternate approach, if appropriate
3. Throw exception.

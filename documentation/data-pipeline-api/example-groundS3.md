## Ground-side

After the data pipeline has transferred sent files to the ground, they are
stored in S3 buckets, where they can be retrieved for further processing.
The files will be delivered in the original format they were sent in, so
if they were sent compressed, they will be stored compressed.  

The topic the file was sent to determines the specific S3 bucket that a
file will be uploaded to.

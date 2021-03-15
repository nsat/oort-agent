from oort_collector_client import CollectorApi
from oort_collector_client.models import InfoRequest

from pprint import pprint

cl = CollectorApi()

pprint(cl.info(InfoRequest()))


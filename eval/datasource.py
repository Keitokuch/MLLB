from abc import ABC, abstractmethod
import logging
import json
import time

logger = logging.getLogger('datasource')
fhdlr = logging.FileHandler('write.log', mode='w')
fhdlr.terminator=''
logger.addHandler(fhdlr)


class DataSource(ABC):

    @abstractmethod
    def update(self, event):
        pass

    @abstractmethod
    def dump(self):
        pass


class FuncLatencyDatasource(DataSource):
    def __init__(self, write_file, append=True):
        self.write_file = write_file
        self.entries = []
        #  if not append:
        #      with open(self.write_file, 'w') as f:
        #          f.write(','.join(self.columns) + '\n')

        print('FuncLatency Datasource initialized. Writing to {}'.format(write_file))
        print('appending: {}'.format(append))
        if append:
            try:
                with open(write_file, 'r') as f:
                    self.entries = json.load(f)
                print(f'{len(self.entries)} Entries loaded from file {write_file}')
            except Exception as e:
                print(e)
                print(f"Failed to load json file {write_file}")
        self.start_time = time.time()
        self.start_len = len(self.entries)

    def update(self, event):
        self.entries.append(event.delta)

    def dump(self, filename=None):
        filename = filename or self.write_file or 'output.json'
        with open(self.write_file, mode='w') as f:
            json.dump(self.entries, f)
            #  for row in self.entries:
            #      f.write(','.join(row) + '\n')
        print(f'{len(self.entries)} Entries written to file {self.write_file}')
        delta_len = len(self.entries) - self.start_len
        delta_time = time.time() - self.start_time
        print(f'{delta_len} Entries collected in {delta_time:.2f}s. {delta_len/delta_time:.1f}/s')
        self.entries = []
        return 'Entries Dumped'


class MaxImbalanceDatasource(DataSource):
    def __init__(self, write_file, append=True):
        self.write_file = write_file
        self.entries = []

        print('MaxImbalance Datasource initialized. Writing to {}'.format(write_file))
        print('appending: {}'.format(append))
        if append:
            try:
                with open(write_file, 'r') as f:
                    self.entries = json.load(f)
                print(f'{len(self.entries)} Entries loaded from file {write_file}')
            except Exception as e:
                print(e)
                print(f"Failed to load json file {write_file}")
        self.start_time = time.time()
        self.start_len = len(self.entries)

    def update(self, imbalance):
        self.entries.append(imbalance)

    def dump(self, filename=None):
        filename = filename or self.write_file or 'output.json'
        with open(self.write_file, mode='w') as f:
            json.dump(self.entries, f)
            #  for row in self.entries:
            #      f.write(','.join(row) + '\n')
        print(f'{len(self.entries)} Entries written to file {self.write_file}')
        delta_len = len(self.entries) - self.start_len
        delta_time = time.time() - self.start_time
        print(f'{delta_len} Entries collected in {delta_time:.2f}s. {delta_len/delta_time:.1f}/s')
        self.entries = []
        return 'Entries Dumped'


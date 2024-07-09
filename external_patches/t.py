import pandas as pd

data = pd.read_csv('Pd-patch-examples.csv')

# Create the 'Instruction' column based on 'Description' and 'Keywords'
data['instruction'] = 'create a purr-data patch that matches the following description and keywords.'
data['input'] = ' Description: ' + data['Description'] + ', Keywords: ' + data['Keywords']

data.to_csv('Pd-patch-examples-inst-resp.csv', index=False)

print(data.head())

import pandas as pd
import matplotlib.pyplot as plt
import os 
import re

# Read the three files
withoutMain = pd.read_csv('mso_without_backup.csv', header=None, names=['test', "timeWithoutMain", "err"])
withoutBackup = pd.read_csv('mso_without_main.csv', header=None, names=['test', "timeWithoutBackup", "err"])
full = pd.read_csv('results-mso.csv', header=None, names=['test', "timeWithUnreliable", "err"])
# Filter out tests that do not have 'hiker' in test name
merged = pd.merge(withoutMain, withoutBackup, on='test').merge(full, on='test')
#merged = merged[merged['test'].str.contains('hiker')]
# Sort by alphanum key
def alphanum_key(s):
    """Turn a string into a list of string and number chunks.
    """
    return [int(text) if text.isdigit() else text for text in re.split('([0-9]+)', s)]

merged = merged.sort_values(by='test', key=lambda col: col.map(alphanum_key))

# Remove rows where any of the time values is -1
merged = merged[(merged['timeWithoutMain'] != -1) & (merged['timeWithoutBackup'] != -1) & (merged['timeWithUnreliable'] != -1)]
# Match the columns
merged['test'] = merged['test'].apply(lambda x: os.path.basename(x))
#plt.subplots_adjust(bottom=0.3)

ax = merged[['test', 'timeWithoutMain', 'timeWithoutBackup', 'timeWithUnreliable']].plot.bar(x='test', rot=70, figsize=(15, 15))

# Adjust subplot parameters for more space
plt.subplots_adjust(bottom=0.3)  # Adjust bottom as needed

# Optionally, set rotation and alignment for better readability
ax.set_xticklabels(ax.get_xticklabels(), rotation=70, ha='right')
#plt.show()
plt.savefig('performance_comparison.png')
print(merged)

# Compute performance penalties
merged['penaltyWithoutMain'] = (merged['timeWithUnreliable']) / merged['timeWithoutMain'] 
merged['penaltyWithoutBackup'] = (merged['timeWithUnreliable']) / merged['timeWithoutBackup'] 
merged['penaltySum'] = (merged['timeWithUnreliable']) /  (merged['timeWithoutBackup'] + merged['timeWithoutMain']) 

# Calculate mean performance penalties
mean_penalty_without_main = merged['penaltyWithoutMain'].mean()
mean_penalty_without_backup = merged['penaltyWithoutBackup'].mean()
mean_penalty_both = merged['penaltySum'].mean()

# Calculate median performance penalties
median_penalty_without_main = merged['penaltyWithoutMain'].median()
median_penalty_without_backup = merged['penaltyWithoutBackup'].median()
median_penalty_both = merged['penaltySum'].median()
# Calculate standard deviation for penalties
std_penalty_without_main = merged['penaltyWithoutMain'].std()
std_penalty_without_backup = merged['penaltyWithoutBackup'].std()
std_penalty_both = merged['penaltySum'].std()

# Print the results 
# Plot mean performance penalties
penalties = pd.DataFrame({
    'Penalty for main': merged['penaltyWithoutMain'],
    'Penalty for backup': merged['penaltyWithoutBackup'],
    'Penalty for  both': merged['penaltySum']
})
boxplot = penalties.plot(kind='box', figsize=(10,10))
plt.title('Performance Comparison')

for i, column in enumerate(penalties.columns):
    y = penalties[column]
    # Calculate Q1 (25th percentile) and Q3 (75th percentile)
    Q1 = y.quantile(0.25)
    Q3 = y.quantile(0.75)
    # Calculate the Interquartile Range (IQR)
    IQR = Q3 - Q1
    # Calculate the whiskers
    lower_whisker = Q1 - 1.5 * IQR
    upper_whisker = Q3 + 1.5 * IQR
    # Identify outliers
    outliers = y[(y < lower_whisker) | (y > upper_whisker)]
    
    for j in outliers.index:
        test_name = merged.loc[j, 'test']
        #boxplot.annotate(test_name, xy=(i + 1, y[j]), xytext=(i + 1.05, y[j]),
        #         ha='left', va='center', fontsize=10, rotation=0)

# Plot box plot for performance penalties
plt.ylabel('Performance Penalty (performance ratio)')
plt.savefig('boxplot_labelled.png')
#plt.show()

print("Mean Performance Penalties:")
print("Penalty without main:", mean_penalty_without_main)
print("Penalty without backup:", mean_penalty_without_backup)
print("Penalty for both:", mean_penalty_both)

print("\nMedian Performance Penalties:")
print("Penalty without main:", median_penalty_without_main)
print("Penalty without backup:", median_penalty_without_backup)
print("Penalty for both:", median_penalty_both)

# Print standard deviation for penalties
print("\nStandard Deviation for Performance Penalties:")
print("Standard Deviation for penalty without main:", std_penalty_without_main)
print("Standard Deviation for penalty without backup:", std_penalty_without_backup)
print("Standard Deviation for penalty for both:", std_penalty_both)

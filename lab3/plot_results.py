import pandas as pd
import matplotlib.pyplot as plt

# Read CSV file

def estimated_IO_cost(moviesSelectivity, bufferSize):
    workedOnSelectivity = 0.111611
    moviesPages = 962
    workedOnPages = 962
    peoplePages = 2858
    joinPages = workedOnSelectivity * workedOnPages * 49 / 4096
    IOCost = workedOnPages + workedOnSelectivity * workedOnPages + \
             moviesPages + moviesPages / ((bufferSize - 6) / 2) * workedOnSelectivity * workedOnPages + \
             moviesSelectivity * moviesPages + moviesSelectivity * moviesPages / ((bufferSize - 6) / 2) * peoplePages
    return IOCost

def plot_results(filename, bufferSize):
    # Read the CSV file
    df = pd.read_csv(filename)

    # Plot execution times
    measured_IO_cost = df['IOCount']
    estimated_IO_costs = [estimated_IO_cost(selectivity, bufferSize) for selectivity in df['selectivity']]
    plt.figure()
    plt.plot(df['selectivity'], measured_IO_cost, marker='o', label='Measured IO Cost')
    plt.plot(df['selectivity'], estimated_IO_costs, marker='s', label='Estimated IO Cost')
    plt.xlabel("Selectivity (fraction of rows selected)")
    plt.ylabel("IO Cost")
    plt.title(f"IO Cost vs. Selectivity (bufferSize = {bufferSize})")
    plt.legend()
    plt.savefig(f"IO_cost_comparison_bufferSize_{bufferSize}.png")
    plt.show()

if __name__ == "__main__":
    # Example usage
    plot_results("build/performance_test.csv", 20)


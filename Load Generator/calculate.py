import datetime as datetime
import os
import matplotlib
matplotlib.use('Agg') 
import matplotlib.pyplot as plt
import concurrent.futures

def generate_unique_filename(filename):
  counter = 1
  while True:
    new_filename = f"{filename}_{counter}.txt"
    if not os.path.exists(new_filename):
      return new_filename
    counter += 1

# Function to parse the timestamp and calculate the time difference in milliseconds
def calculate_time_difference(begin_time, end_time):
    begin_timestamp = datetime.datetime.strptime(begin_time, "%H:%M:%S.%f")
    end_timestamp = datetime.datetime.strptime(end_time, "%H:%M:%S.%f")
    time_diff = (end_timestamp - begin_timestamp).total_seconds() * 1000  # Convert to milliseconds
    return time_diff

# Open the file for writing
with open(generate_unique_filename('output'), 'w') as output_file:

    # Read the log.txt file and process each entry
    with open('log.txt', 'r') as log_file:
        lines = log_file.readlines()

    # Initialize variables to store the current token and begin time
    current_token = None
    current_type = None
    current_begin_time = None
    request_times = []
    mainserver_times = []
    redis_times = []
    discovery_times = []

    # Define the list of valid types
    valid_types = ['discovery', 'mainserver', 'redis', 'request']

    # Process each line in the log file
    for line in lines:
        line = line.strip()

        # Check for the token line and extract the token value
        if line.startswith('token: '):
            if current_token:
                output_file.write('\n')
            current_token = line.split(' ')[1]
            output_file.write(f"Token {current_token}:\n")

        # Check for valid types and calculate and print time differences 
        for valid_type in valid_types:
            if line.startswith(f'{valid_type}_begin_time:'):
                current_type = valid_type
                current_begin_time = line.split(' ')[1]
            elif line.startswith(f'{valid_type}_end_time:'):
                end_time = line.split(' ')[1]
                if current_type and current_begin_time:
                    time_difference = calculate_time_difference(current_begin_time, end_time)
                    output_file.write(f"Type: {current_type.capitalize()} - Processing Time = {time_difference:.2f} ms\n")
                    if current_type == 'request':
                        request_times.append((time_difference, current_token))
                    if current_type == 'mainserver':
                        mainserver_times.append((time_difference, current_token))
                    if current_type == 'redis':
                        redis_times.append((time_difference, current_token))
                    if current_type == 'discovery':
                        discovery_times.append((time_difference, current_token))

    output_file.write('\n')
    output_file.write("Summary:\n")
    output_file.write(f"Total tokens:{len(request_times)}\n")
    output_file.write('\n')

    if(request_times):
        output_file.write("Total Time\n")
        min_time = min(request_times, key=lambda x: x[0])
        max_time = max(request_times, key=lambda x: x[0])
        avg_time = sum(item[0] for item in request_times) / len(request_times)

        # Calculate the median of the first values (time values)
        sorted_times = sorted(request_times, key=lambda x: x[0])
        middle = len(sorted_times) // 2
        if len(sorted_times) % 2 == 0:
            median_time = (sorted_times[middle - 1][0] + sorted_times[middle][0]) / 2
            median_tokens = [sorted_times[middle - 1][1], sorted_times[middle][1]]
        else:
            median_time = sorted_times[middle][0]
            median_tokens = [sorted_times[middle][1]]
            

        # Calculate the 95th percentile of the first values (time values)
        percentile_95 = sorted_times[int(len(sorted_times) * 0.95)][0]
        percentile_95_tokens = [item[1] for item in sorted_times if item[0] >= percentile_95]
        percentile_95_tokens.sort()

        # Print the corresponding second elements
        output_file.write(f"Minimum Time: {min_time[0]:.3f} ms (Token {min_time[1]})\n")
        output_file.write(f"Maximum Time: {max_time[0]:.3f} ms (Token {max_time[1]})\n")
        output_file.write(f"Average Time: {avg_time:.3f} ms\n")
        output_file.write(f"Median Time: {median_time:.3f} ms\n")
        output_file.write(f"Median Tokens: {', '.join(median_tokens)}\n")
        output_file.write(f"95th Percentile Time: {percentile_95:.3f} ms\n")
        # output_file.write(f"95th Percentile Tokens: {', '.join(percentile_95_tokens)}\n")
        output_file.write('\n')

    if(mainserver_times):
        output_file.write("Main Server Time")
        output_file.write('\n')
        min_time = min(mainserver_times, key=lambda x: x[0])
        max_time = max(mainserver_times, key=lambda x: x[0])
        avg_time = sum(item[0] for item in mainserver_times) / len(mainserver_times)

        # Calculate the median of the first values (time values)
        sorted_times = sorted(mainserver_times, key=lambda x: x[0])
        middle = len(sorted_times) // 2
        if len(sorted_times) % 2 == 0:
            median_time = (sorted_times[middle - 1][0] + sorted_times[middle][0]) / 2
            median_tokens = [sorted_times[middle - 1][1], sorted_times[middle][1]]
        else:
            median_time = sorted_times[middle][0]
            median_tokens = [sorted_times[middle][1]]
            

        # # Calculate the 95th percentile of the first values (time values)
        percentile_95 = sorted_times[int(len(sorted_times) * 0.95)][0]
        percentile_95_tokens = [item[1] for item in sorted_times if item[0] >= percentile_95]
        percentile_95_tokens.sort()

        # Print the corresponding second elements
        output_file.write(f"Minimum Time: {min_time[0]:.3f} ms (Token {min_time[1]})\n")
        output_file.write(f"Maximum Time: {max_time[0]:.3f} ms (Token {max_time[1]})\n")
        output_file.write(f"Average Time: {avg_time:.3f} ms\n")
        output_file.write(f"Median Time: {median_time:.3f} ms\n")
        output_file.write(f"Median Tokens: {', '.join(median_tokens)}\n")
        output_file.write(f"95th Percentile Time: {percentile_95:.3f} ms\n")
        # output_file.write(f"95th Percentile Tokens: {', '.join(percentile_95_tokens)}\n")
        output_file.write('\n')
    
    if(redis_times):
        output_file.write("Redis Server Time")
        output_file.write('\n')
        min_time = min(redis_times, key=lambda x: x[0])
        max_time = max(redis_times, key=lambda x: x[0])
        avg_time = sum(item[0] for item in redis_times) / len(redis_times)

        # Calculate the median of the first values (time values)
        sorted_times = sorted(redis_times, key=lambda x: x[0])
        middle = len(sorted_times) // 2
        if len(sorted_times) % 2 == 0:
            median_time = (sorted_times[middle - 1][0] + sorted_times[middle][0]) / 2
            median_tokens = [sorted_times[middle - 1][1], sorted_times[middle][1]]
        else:
            median_time = sorted_times[middle][0]
            median_tokens = [sorted_times[middle][1]]
            

        # # Calculate the 95th percentile of the first values (time values)
        percentile_95 = sorted_times[int(len(sorted_times) * 0.95)][0]
        percentile_95_tokens = [item[1] for item in sorted_times if item[0] >= percentile_95]
        percentile_95_tokens.sort()

        # Print the corresponding second elements
        output_file.write(f"Minimum Time: {min_time[0]:.3f} ms (Token {min_time[1]})\n")
        output_file.write(f"Maximum Time: {max_time[0]:.3f} ms (Token {max_time[1]})\n")
        output_file.write(f"Average Time: {avg_time:.3f} ms\n")
        output_file.write(f"Median Time: {median_time:.3f} ms\n")
        output_file.write(f"Median Tokens: {', '.join(median_tokens)}\n")
        output_file.write(f"95th Percentile Time: {percentile_95:.3f} ms\n")
        # output_file.write(f"95th Percentile Tokens: {', '.join(percentile_95_tokens)}\n")
        output_file.write('\n')

    if(discovery_times):
        output_file.write("Discovery Server Time")
        output_file.write('\n')
        min_time = min(discovery_times, key=lambda x: x[0])
        max_time = max(discovery_times, key=lambda x: x[0])
        avg_time = sum(item[0] for item in discovery_times) / len(discovery_times)

        # Calculate the median of the first values (time values)
        sorted_times = sorted(discovery_times, key=lambda x: x[0])
        middle = len(sorted_times) // 2
        if len(sorted_times) % 2 == 0:
            median_time = (sorted_times[middle - 1][0] + sorted_times[middle][0]) / 2
            median_tokens = [sorted_times[middle - 1][1], sorted_times[middle][1]]
        else:
            median_time = sorted_times[middle][0]
            median_tokens = [sorted_times[middle][1]]
            

        # Calculate the 95th percentile of the first values (time values)
        percentile_95 = sorted_times[int(len(sorted_times) * 0.95)][0]
        percentile_95_tokens = [item[1] for item in sorted_times if item[0] >= percentile_95]
        percentile_95_tokens = sorted(map(int, percentile_95_tokens))

        # Print the corresponding second elements
        output_file.write(f"Minimum Time: {min_time[0]:.3f} ms (Token {min_time[1]})\n")
        output_file.write(f"Maximum Time: {max_time[0]:.3f} ms (Token {max_time[1]})\n")
        output_file.write(f"Average Time: {avg_time:.3f} ms\n")
        output_file.write(f"Median Time: {median_time:.3f} ms\n")
        output_file.write(f"Median Tokens: {', '.join(median_tokens)}\n")
        output_file.write(f"95th Percentile Time: {percentile_95:.3f} ms\n")
        # output_file.write(f"95th Percentile Tokens: {', '.join(map(str, percentile_95_tokens))}\n")
        output_file.write('\n')

# Extract x and y values for each dataset
# request_time, request_token = zip(*request_times)
# mainserver_time, mainserver_token = zip(*mainserver_times)
# redis_time, redis_token = zip(*redis_times)
# discovery_time, discovery_token = zip(*discovery_times)

# # Create a line graph
# plt.figure(figsize=(20, 12))
# plt.plot(request_token, request_time, marker='o', label='Request Times', linestyle='-')
# plt.plot(mainserver_token, mainserver_time, marker='o', label='Main Server Times', linestyle='-')
# plt.plot(redis_token, redis_time, marker='o', label='Redis Server Times', linestyle='-')
# plt.plot(discovery_token, discovery_time, marker='o', label='Discovery Server Times', linestyle='-')

# # Add labels and legend
# plt.xlabel('Token')
# plt.ylabel('Time (ms)')
# plt.title('Time vs Token')
# plt.legend()

# # Show the plot
# plt.grid(True)
# plt.tight_layout()
# plt.savefig("response_time_graph.png")
# plt.close()
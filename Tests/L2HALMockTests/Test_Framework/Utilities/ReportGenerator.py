#** *****************************************************************************
# *
# * If not stated otherwise in this file or this component's LICENSE file the
# * following copyright and licenses apply:
# *
# * Copyright 2024 RDK Management
# *
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *
# http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *
#* ******************************************************************************

import csv
import os
import pandas as pd
from bs4 import BeautifulSoup

passed_tc_list = []
failed_tc_list = []


def append_test_results_to_csv(tc_id, output, status, message):

    # Assign the headers for report file
    headers = ["Curl API/TC Name", "Output Response", " TC Status ", "  Remarks  "]

    # Open the report file in append mode
    with open("TestReport.csv", "a", newline="") as csvfile:
        writer = csv.writer(csvfile)

        # Write headers if the headers in report file is empty
        if csvfile.tell() == 0:
            writer.writerow(headers)

        # Write the testcase details to a new row
        writer.writerow([tc_id, output, status, message])



# Report Generation in .html file format
def generate_html_report(build_name, now):

    # To read csv file named "TestReport"
    a = pd.read_csv("TestReport.csv")

    # Format the date and time as strings to generate the test report file
    date = now.strftime("%d-%m-%Y")
    time = now.strftime("%Hh%Mm%Ss")

    # To save as html file with build name, execution data and time
    a.to_html("TestReport_{}_{}_{}.html".format(build_name, date, time), index=False)
    # To remove the TestReport.csv file
    #os.remove("TestReport.csv")

    # Add custom CSS style for a black background
    # Read the HTML file
    with open("TestReport_{}_{}_{}.html".format(build_name, date, time), "r") as f:
        html = f.read()

    styled_html = f"""
    <style>
        table {{
            background-color: black;
            color: white;
        }}
    </style>
    {html}
    """

    # Parse the HTML using BeautifulSoup
    soup = BeautifulSoup(html, "html.parser")

    # Find the table element
    table = soup.find("table")

    # Create a list of elements with the data

    # Calculate the length of the failed_tc_list string
    failed_tc_length = str(len(failed_tc_list))
    passed_tc_length = str(len(passed_tc_list))
    
    # Create a formatted string with the length in red
    #formatted_length_failed = f"\033[91m{failed_tc_length}\033[0m"
    #formatted_length_passed = f"\033[92m{passed_tc_length}\033[0m"
    formatted_length_failed = failed_tc_length
    formatted_length_passed = passed_tc_length
  
    current_date = now.strftime("%d/%m/%Y")
    current_time = now.strftime("%H:%M:%S")
    data = [soup.new_tag("p")]
    # Now you can use the formatted_length wherever you need it
    ##data[-1].string = f"Number of Testcases Failed: {str(formatted_length_failed)}"
    #data[-1].string = f"Number of Testcases Failed: {str(len(failed_tc_list))}"
    ##data.append(soup.new_tag("p"))
    #data[-1].string = f"Number of Testcases Passed: {str(len(passed_tc_list))}"
    ##data[-1].string = f"Number of Testcases Passed: {str(formatted_length_passed)}"
    ##data.append(soup.new_tag("p"))
    ##data[-1].string = f"Time of Execution: {current_time}"
    ##data.append(soup.new_tag("p"))
    ##data[-1].string = f"Date of Execution: {current_date}"
    ##data.append(soup.new_tag("p"))
    ##data[-1].string = f"Build Name: {build_name}"

    styled_html = f"""
        <html>
        <head>
            <style>
                body {{
                    background-color: black;
                    color: white;
                    font-family: Arial, sans-serif;
                }}
                table {{
                    border-collapse: collapse;
                    width: 200%;
                    margin-bottom: 20px;
                }}
                th, td {{
                    border: 3px solid white;
                    padding: 8px;
                    text-align: left;
                }}
                 th:nth-child(1), td:nth-child(1) {{
                    width: 0%;  /* Adjust the width as needed */
                }}
                th:nth-child(4), td:nth-child(4) {{
                    width: 200%;  /* Adjust the width as needed */
                }}
                th {{
                    background-color: #333;
                }}
            </style>
        </head>
        <body>
            <h1>L2 Test Execution Report</h1>
            {html}
            <p>Execution Summary</p>
            <p>Number of Testcases Failed: {str(formatted_length_failed)}</p>
            <p>Number of Testcases Passed: {str(formatted_length_passed)}</p>
            <p>Time of Execution: {current_time}</p>
            <p>Date of Execution: {current_date}</p>
            <p>Build Name: {build_name}</p>
        </body>
        </html>
        """


    # Insert the data elements before the table element
    for element in reversed(data):
        table.insert_before(element)


    # Replace the old html file with new one
    with open("TestReport_{}_{}_{}.html".format(build_name, date, time), "w") as f:
        f.write(str(soup))

    # Read the html file
    with open("TestReport_{}_{}_{}.html".format(build_name, date, time)) as html_file:
        soup = BeautifulSoup(html_file, "html.parser")

    # Find all the <th> elements
    headers = soup.find_all("th")

    # Add the style attribute to each <th> element
    for header in headers:
        header["style"] = "text-align: center"


    # Write the modified html to a same report file
    with open("TestReport_{}_{}_{}.html".format(build_name, date, time), "w") as html_file:
        #html_file.write(soup.prettify())
        html_file.write(styled_html)
    # Move the html file to the 'Test Execution Reports' Directory
    os.rename("TestReport_{}_{}_{}.html".format(build_name, date, time),
              "Test Execution Reports/TestReport_{}_{}_{}.html".format(
                  build_name, date, time))

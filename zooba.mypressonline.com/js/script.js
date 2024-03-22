function checkFile(input) {
    var uploadBtn = document.getElementById('uploadBtn');

    if (input.files && input.files.length > 0) {
      // At least one file is selected, enable the button
      uploadBtn.classList.remove('inactive');
    } else {
      // No file selected, disable the button
      uploadBtn.classList.add('inactive');
    }   
     
    // Get the file input element
    var fileInput = document.getElementById('file');

    // Check if any file is selected
    if (fileInput.files.length > 0) {
        // Get the selected file
        var selectedFile = fileInput.files[0];
        var name = fileInput.files[0].name;
        var fileName = document.getElementById('fileName');
        //fileName.textContent = name;
        // Read the content of the file
        var reader = new FileReader();
        reader.onload = function (e) {
            // Display the content in the 'fileContent' div
            var fileContentDiv = document.getElementById('fileContent');
            //fileContentDiv.textContent = e.target.result;
            //fileContentDiv.style.height = 400
        };

        // Read the file as text
        reader.readAsText(selectedFile);
    } else {
        // If no file is selected, clear the content
        var fileContentDiv = document.getElementById('fileContent');
        var fileName = document.getElementById('fileName');
        fileContentDiv.textContent = '';
        fileContentDiv.style.height = 0;
        fileName.textContent = '';
    }
}

function performGetRequest() {
  var resultElement = document.getElementById('getResult');
  resultElement.innerHTML = '';

  axios.get('http://localhost:8000/')
    .then(function (response) {
      resultElement.innerHTML = generateSuccessHTMLOutput(response);
    })
    .catch(function (error) {
      resultElement.innerHTML = generateErrorHTMLOutput(error);
    });
}

document.getElementById('todoInputForm').addEventListener('submit', performPostRequest); function performPostRequest(e) {
  var resultElement = document.getElementById('postResult');
  var todoTitle = document.getElementById('todoTitle').value;
  resultElement.innerHTML = '';

  axios.post('http://localhost:8000/upload', {
    data: todoTitle,
  })
    .then(function (response) {
      resultElement.innerHTML = generateSuccessHTMLOutput(response);
    })
    .catch(function (error) {
      resultElement.innerHTML = generateErrorHTMLOutput(error);
    });

  e.preventDefault();
}

function generateSuccessHTMLOutput(response) {
  console.log(response);
  return '<h4>Result</h4>' +
    '<h5>Status:</h5> ' +
    '<pre>' + response.status + ' ' + response.statusText + '</pre>' +
    '<h5>Headers:</h5>' +
    '<pre>' + JSON.stringify(response.headers, null, '\t') + '</pre>' +
    '<h5>Data:</h5>' +
    '<pre>' + JSON.stringify(response.data, null, '\t') + '</pre>';
}

function generateErrorHTMLOutput(error) {
  return '<h4>Result</h4>' +
    '<h5>Message:</h5> ' +
    '<pre>' + error.message + '</pre>' +
    '<h5>Status:</h5> ' +
    '<pre>' + error.response.status + ' ' + error.response.statusText + '</pre>' +
    '<h5>Headers:</h5>' +
    '<pre>' + JSON.stringify(error.response.headers, null, '\t') + '</pre>' +
    '<h5>Data:</h5>' +
    '<pre>' + JSON.stringify(error.response.data, null, '\t') + '</pre>';
}

function clearOutput() {
  var resultElement = document.getElementById('getResult');
  resultElement.innerHTML = '';
  var resultElement = document.getElementById('postResult');
  resultElement.innerHTML = '';
}
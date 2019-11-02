import axios from 'axios';

const BASE_URL = 'https://jsonplaceholder.typicode.com';

const getTodos = async () => {
    try {
        const res = await axios.get(`${BASE_URL}/todos`);

        const todos = res.data;

        console.log(`GET: Here's the list of todos`, todos);

        return todos;
    } catch (e) {
        console.error(e);
    }
};

const createLi = item => {
    const li = document.createElement('li');

    li.appendChild(document.createTextNode(item.title));

    return li;
};

const addTodosToDOM = todos => {
    const ul = document.querySelector('ul');

    if (Array.isArray(todos) && todos.length > 0) {
        todos.map(todo => {
            ul.appendChild(createLi(todo));
        });
    } else if (todos) {
        ul.appendChild(createLi(todos));
    }
};

const main = async () => {
    addTodosToDOM(await getTodos());
};

const form = document.querySelector('form');

const formEvent = form.addEventListener('submit', async event => {
    event.preventDefault();

    const title = document.querySelector('#new-todos__title').value;
    const userId = document.querySelector('#new-todos__userId').value;

    const todo = {
        title,
        userId
    };

    const addedTodo = await addTodo(todo);
    addTodosToDOM(addedTodo);
});

export const addTodo = async todo => {
    try {
        const res = await axios.post(`${BASE_URL}/todos`, todo);
        const addedTodo = res.data;

        console.log(`Added a new Todo!`, addedTodo);

        return addedTodo;
    } catch (e) {
        console.error(e);
    }
};



main();  
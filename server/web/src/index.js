import Vue from 'vue'
import app from './components/app'
import store from './store/'

new Vue({
    el: '#app',
    store,
    render: h => h(app)
});

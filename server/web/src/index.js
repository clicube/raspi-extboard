import Vue from 'vue'

// import Vuetify from 'vuetify'
// import 'vuetify/dist/vuetify.min.css'

// Vue.use(Vuetify)

import app from './components/app'
import store from './store/'

new Vue({
    el: '#root',
    store,
    components: { app },
    template: '<app/>',
})

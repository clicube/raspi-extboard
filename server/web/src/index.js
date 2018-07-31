import Vue from 'vue'
import Vuetify from 'vuetify'
import 'vuetify/dist/vuetify.min.css'

Vue.use(Vuetify)

console.log("a")

import app from './components/app'

new Vue({
    el: '#app',
    components: {app},
    template: '<app/>',
})


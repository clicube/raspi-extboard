import Vue from 'vue'
import Vuetify from 'vuetify'
import 'vuetify/dist/vuetify.min.css'

Vue.use(Vuetify)

import app from './components/app'

new Vue({
    el: '#root',
    components: {app},
    template: '<app/>',
})


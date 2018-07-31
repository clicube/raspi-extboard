<template>
    <div>
        <v-slide-y-transition name="fade">
            <div v-if="isControlsVisible">
                <v-card class="controlCard">
                    <v-card-title primary-title>
                        <h3 class="headline mb-0">Air Conditioner</h3>
                    </v-card-title>
                    <v-card-actions>
                        <v-btn flat color="orange">ON</v-btn>
                        <v-btn flat color="blue">OFF</v-btn>
                    </v-card-actions>
                </v-card>
                <v-card class="controlCard">
                    <h2 class="headline">ライト</h2>
                    <v-btn large>ON</v-btn>
                    <v-btn large>OFF</v-btn>
                </v-card>
            </div>
        </v-slide-y-transition>
        <v-btn fab dark id="floatingButton" @click="onRemoconButtonClick">
            <v-icon dark>settings_remote</v-icon>
        </v-btn>
    </div>
</template>

<script>
    export default {
        data: () => {
            return {
                isControlsVisible: false
            }
        },
        methods: {
            onRemoconButtonClick: function () {
                this.isControlsVisible = !this.isControlsVisible
            }
        }
    }
</script>

<style scoped>
    #floatingButton {
        position: fixed;
        right: 20px;
        bottom: 20px;
    }

    .controlCard {
        padding: 10px;
        margin-bottom: 10px;
    }

</style>
